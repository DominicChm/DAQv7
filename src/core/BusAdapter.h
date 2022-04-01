//
// Created by Dominic on 3/29/2022.
//

#ifndef DAQV7_BUSADAPTER_H
#define DAQV7_BUSADAPTER_H

#include <Arduino.h>

#define PACKETIZER_BUFFER_SIZE 255

#define HEADER_MAGIC_START 0x7E
#define HEADER_SIZE 4
#define TRAILER_SIZE 2

#define PARSER_TIMEOUT_MS 0
#define BUS_TIMEOUT_MS 0

#define HEADER_INDEX_ADDRESS 1
#define HEADER_INDEX_SIZE_LSB 2
#define HEADER_INDEX_SIZE_MSB 3

#define TRAILER_INDEX_CHECK_LSB 0
#define TRAILER_INDEX_CHECK_MSB 1


enum ParseState {
    IDLE,
    PARSE_HEADER,
    PARSE_PAYLOAD,
    PARSE_TRAILER, // Apparently the end of a packet is called a "trailer"... Who knew?
};


struct BusOptions {
    uint64_t parser_timeout_ms;
    uint64_t lock_timeout_ms;
};

/**
 * Handles parsing and encoding of packets on the bus.
 * Also handles control flow.
 */
template<size_t buffer_size = 256>
class BusAdapter {
//private:
public:
    // Set at construction time as the number of free bytes in the serial tx buffer.
    // Manually writing to the bus before initializing the adapter will cause this to be off!!
    size_t hw_tx_buffer_size = 0;

    Stream &serial;
    const uint8_t driver_enable_pin;
    uint8_t address;
    uint32_t baudrate;
    BusOptions opts;


    ParseState parseState = IDLE;

    struct PacketHeader {
        size_t data_size;
        bool response_expected;
        uint8_t address;
    } packet_header;

    struct PacketTrailer {
        uint16_t checksum;
    } packet_trailer;

    // Performance Tracking
    uint32_t num_oversized_packets = 0;

    // Bus parseState variables
    bool bus_locked;
    uint64_t driver_disable_delay;
    uint64_t lock_started_ms;

    // The bus's lock parseState after a transmission has finished. Set by the transmitting method.
    // IE a response packet will unlock the bus once finished, while a response requesting packet will lock it.
    bool post_transmit_lock;
    bool transmit_finalized;

    // Parse variables
    size_t packet_idx = 0;
    uint64_t parse_started_ms = 0;
    uint64_t finalization_started_us = 0;

    // Header FSM vars
    uint8_t header_buf[HEADER_SIZE];

    // TX buffer
    uint8_t transmit_buf[buffer_size];
    uint8_t transmit_size;
    uint8_t transmit_idx;

    // Data buffer
    uint8_t receive_buf[buffer_size];
    size_t receive_head;
    size_t receive_tail; //Used for reading the FIFO cache.

    void do_finalize_transmit() {
        if (transmit_finalized || is_transmitting()) return;

        transmit_finalized = true;
        bus_locked = post_transmit_lock;
        finalization_started_us = millis();
    }

    void do_transmit() {
        if (!is_buffering_transmit()) return;

        if (hw_tx_buffer_size > 0) {
            // We have a non-zero tx buffer. Fill it.
            while (serial.availableForWrite() && is_buffering_transmit())
                serial.write(transmit_buf[transmit_idx++]);

        } else {
            // There's no hw buffer. Fallback to 1-by-1 writes.
            serial.write(transmit_buf[transmit_idx++]);
        }
    }

    // Delays disabling of drivers for one extra serial byte.
    // Disabling early chops the last byte off the packet.
    void do_disable_drivers() {
        if (finalization_started_us == 0 || millis() - finalization_started_us < driver_disable_delay) return;

        digitalWrite(driver_enable_pin, LOW);
        finalization_started_us = 0;
    }

    void handle_new_data() {
        //Figure out bus status
        //Lock on all response_expected packets, except for when it's a response.
        bus_locked = packet_header.response_expected && packet_header.address != 0x7F;
        if (bus_locked) lock_started_ms = millis();
    }

    bool parser_timed_out() {
        return parseState != IDLE && opts.parser_timeout_ms != 0 && millis() - parse_started_ms > PARSER_TIMEOUT_MS;
    }

    bool lock_timed_out() {
        return bus_locked && opts.lock_timeout_ms != 0 && (millis() - lock_started_ms > BUS_TIMEOUT_MS);
    }

    void init_parser(bool reset_data = true) {
        parseState = IDLE;
        packet_idx = 0;
        parse_started_ms = millis();

        if (reset_data) {
            packet_header = {false, 0, 0}; // Reset header info
            packet_trailer = {0};
            receive_head = 0;
            receive_tail = 0;
        }
    }

    void reset_lock() {
        bus_locked = false;
    }

    void packet_parse_fsm(uint8_t b) {
        //Serial.write(b);
        switch (parseState) {
            case IDLE:
                if (b == HEADER_MAGIC_START) {
                    // Magic byte received - packet start.
                    init_parser(true);
                    parse_started_ms = millis();
                    parseState = PARSE_HEADER;
                }
                break;

            case PARSE_HEADER:
                if (header_parse_fsm(b)) {
                    // Skip packets that are too big for our buffer.
                    if (packet_header.data_size > buffer_size) {
                        num_oversized_packets++;
                        init_parser(true);
                    } else {
                        parseState = PARSE_PAYLOAD;
                    }
                }
                break;

            case PARSE_PAYLOAD:
                receive_buf[receive_head++] = b;
                if (receive_head >= packet_header.data_size) {
                    parseState = PARSE_TRAILER;
                }
                break;

            case PARSE_TRAILER:
                if (trailer_parse_fsm(b)) {
                    //TODO: CHECK AGAINST CHECKSUM!
                    init_parser(false);
                    handle_new_data();
                }
        }

        packet_idx++;
    }

    // Returns true when the header is successfully received.
    bool header_parse_fsm(uint8_t b) {
        switch (packet_idx) {
            case HEADER_INDEX_ADDRESS:
                packet_header.response_expected = 0x80 & b;
                packet_header.address = 0x7F & b;
                break;
            case HEADER_INDEX_SIZE_LSB:
                packet_header.data_size = b;
                break;
            case HEADER_INDEX_SIZE_MSB:
                packet_header.data_size |= b << 8;
                return true;
        }

        return packet_idx >= HEADER_SIZE;
    }

    // Returns true when the packet trailer is successfully received. End of packet
    bool trailer_parse_fsm(uint8_t b) {
        size_t trailer_idx = packet_idx - packet_header.data_size - HEADER_SIZE;
        switch (trailer_idx) {
            case 0:
                packet_trailer.checksum = b;
                break;

            case 1:
                packet_trailer.checksum |= b << 8;
                return true;

            default: //What happened here??? - Return false to hopefully hang the FSM and discover the problem.
                return false;
        }
        return false;
    }

    // Writes a packet header into the passed buffer
    void generate_header(uint8_t *dest, uint16_t data_size, uint8_t dest_address, bool response_required) {
        dest[0] = HEADER_MAGIC_START;
        dest[HEADER_INDEX_ADDRESS] = (dest_address & 0x7F) | (response_required ? 0x80 : 0x00);
        dest[HEADER_INDEX_SIZE_LSB] = data_size;
        dest[HEADER_INDEX_SIZE_MSB] = data_size >> 8;
    }

    // Writes a packet trailer into the passed buffer
    void generate_trailer(uint8_t *dest, uint8_t *data, uint16_t data_size) {
        dest[TRAILER_INDEX_CHECK_LSB] = 0xAB;
        dest[TRAILER_INDEX_CHECK_MSB] = 0xAB;
    }

    bool write_bus(uint8_t *payload, size_t size, uint8_t dest_address, bool response_required) {
        // Only addresses <= 0x7F valid. 0x7F is a global message
        if (dest_address > 0x7F || is_transmitting()) return false;
        transmit_size = 0;
        transmit_idx = 0;

        generate_header(transmit_buf, size, dest_address, response_required);
        transmit_size += HEADER_SIZE;

        memcpy(&(transmit_buf[transmit_size]), payload, size);
        transmit_size += size;

        generate_trailer(&(transmit_buf[transmit_size]), payload, size);
        transmit_size += TRAILER_SIZE;

        bus_locked = response_required;
        transmit_finalized = false;
        digitalWrite(driver_enable_pin, HIGH);

        return true;
    }

public:
    BusAdapter(Stream &serial, uint8_t driver_enable_pin, uint8_t address, uint32_t baudrate, BusOptions opts = {}) :
            serial(serial),
            driver_enable_pin(driver_enable_pin),
            address(address),
            opts(opts),
            baudrate(baudrate) {
        pinMode(driver_enable_pin, OUTPUT);
        digitalWrite(driver_enable_pin, LOW);
        driver_disable_delay = 1000000 * 8 / baudrate; //BAUDRATE
    }

    BusAdapter(HardwareSerial &serial, uint8_t driver_enable_pin, uint8_t address, uint32_t baudrate,
               BusOptions opts = {}) : BusAdapter((Stream &)serial, driver_enable_pin, address, baudrate, opts) {
        serial.begin(baudrate);
    }

    bool begin() {
        hw_tx_buffer_size = serial.availableForWrite();
        return true;
    }

    void loop() {
        if (serial.available()) packet_parse_fsm(serial.read());

        do_transmit();
        do_finalize_transmit();
        do_disable_drivers();

        // Check and handle timeouts
        if (parser_timed_out()) init_parser(true);

        if (lock_timed_out()) reset_lock();
    }

    size_t available() {
        if (parseState != IDLE) return 0; // Only allow getting receive_buf when not actively parsing new receive_buf.
        return receive_head - receive_tail;
    }

    uint8_t read() {
        uint8_t dat;
        read(&dat, 1);
        return dat;
    }

    size_t read(uint8_t *dest, size_t size) {
        size_t data_unread = receive_head - receive_tail;
        size = min(size, data_unread);

        memcpy(dest, &receive_buf[receive_tail], size);
        receive_tail += size;
        return size;
    }

    // Writes a packet to the bus. Non-blocking writes are performed by default.
    // Byte-by-byte transmission (manual metering) is HANDLED BY THE CLASS!
    bool write(uint8_t *payload, size_t size, uint8_t dest_address, bool response_required) {
        if (bus_locked) return false;
        post_transmit_lock = response_required;

        return write_bus(payload, size, dest_address, response_required);
    }

    bool write_global(uint8_t *src, size_t size) {
        post_transmit_lock = false;

        return write(src, size, 0x7F, false);
    }

    bool respond(uint8_t *src, size_t size) {
        if (!response_request_pending()) return false;
        post_transmit_lock = false;

        // A response is to the global address, with the response bit set.
        return write_bus(src, size, 0x7F, true);
    }

    bool is_transmitting() {
        //Transmitting if still buffering tx data, or if we haven't emptied the serial FIFO.
        return transmit_idx < transmit_size || serial.availableForWrite() < hw_tx_buffer_size;
    }

    bool is_buffering_transmit() {
        return transmit_idx < transmit_size;
    }

    bool response_request_pending() {
        return bus_locked && packet_header.response_expected && packet_header.address == address;
    }

    PacketHeader header() {
        return packet_header;
    }
};

#endif //DAQV7_BUSADAPTER_H
