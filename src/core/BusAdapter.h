//
// Created by Dominic on 3/29/2022.
//

#ifndef DAQV7_BUSADAPTER_H
#define DAQV7_BUSADAPTER_H

#include <Arduino.h>

#define PACKETIZER_BUFFER_SIZE 255

#define MAGIC_START 0x7E
#define HEADER_SIZE 4

#define PARSER_TIMEOUT_MS 0
#define BUS_TIMEOUT_MS 0

#define INDEX_ADDRESS 1
#define INDEX_SIZE_LSB 2
#define INDEX_SIZE_MSB 3


enum ParseState {
    PARSE_HEADER,
    RECEIVE_DATA,
    PARSE_CHECK,
};

/**
 * Handles parsing and encoding of packets on the bus.
 * Also handles control flow.
 */
class BusAdapter {
private:

    void handle_new_data() {
        //Figure out bus status
        //Lock on all response_expected packets, except for when it's a response.
        locked = packet_header.response_expected && packet_header.address != 0x7F;

        if (locked) lock_started = millis();
    }

public:

    Stream &serial;
    ParseState state = PARSE_HEADER;

    uint8_t address;

    //Bus state variables
    bool locked;
    uint64_t lock_started;


    //Parse variables
    size_t packet_idx = 0;
    uint64_t parse_started = 0;

    uint8_t data_buf[255];
    size_t data_buf_head = 0;
    uint16_t data_check = 0;

    struct PacketHeader {
        bool response_expected;
        uint8_t address;
        size_t data_size;
    } packet_header;

    BusAdapter(Stream &serial, uint8_t address) : serial(serial), address(address) {}

    bool begin() {
        return true;
    }

    void loop() {
        if (serial.available()) parseFSM(serial.read());

        if (parser_timed_out()) resetParser(true);


    }

    void resetParser(bool reset_data = true) {
        packet_idx = 0;
        state = PARSE_HEADER;
        if (reset_data) {
            packet_header = {false, 0, 0}; // Reset header info
            data_check = 0;
        }
    }

    bool parser_timed_out() {
#if PARSER_TIMEOUT_MS == 0
        return false;
#else
        return packet_idx != 0 && millis() - parse_started > PARSER_TIMEOUT_MS;
#endif
    }

    bool lock_timed_out() {
#if BUS_TIMEOUT_MS == 0
        return false;
#else
        return locked && (millis() - lock_started > BUS_TIMEOUT_MS);
#endif
    }

    void parseFSM(uint8_t b) {
        //Guard parsing from starting on a non-magic byte
        if (packet_idx == 0 && b != MAGIC_START) return;

        //Init a new packet
        if (packet_idx == 0) {
            data_buf_head = 0;
            resetParser(true);
            parse_started = millis();
        }

        switch (state) {
            case PARSE_HEADER:
                if (headerParseFsm(b)) state = RECEIVE_DATA;
                break;

            case RECEIVE_DATA:
                data_buf[data_buf_head++] = b;
                if (data_buf_head >= packet_header.data_size) state = PARSE_CHECK;
                break;

            case PARSE_CHECK:
                if (checkParseFsm(b)) {
                    //TODO: CHECK AGAINST CHECKSUM!
                    handle_new_data();
                    resetParser(false);
                    return; // Skip packet index addition.
                }

        }

        packet_idx++;
    }

    // Returns true when the header is fully parsed.
    bool headerParseFsm(uint8_t b) {
        switch (packet_idx) {
            case 0: //Magic Byte check handled in main FSM
                break;
            case INDEX_ADDRESS:
                packet_header.response_expected = 0x80 & b;
                packet_header.address = 0x7F & b;
                break;
            case INDEX_SIZE_LSB:
                packet_header.data_size = b;
                break;
            case INDEX_SIZE_MSB:
                packet_header.data_size |= b << 8;
                return true;
        }

        return packet_idx >= HEADER_SIZE;
    }


    bool checkParseFsm(uint8_t b) {
        size_t check_idx = packet_idx - packet_header.data_size - HEADER_SIZE;
        switch (check_idx) {
            case 0:
                data_check = b;
                break;
            case 1:
                data_check |= b << 8;
                //Serial.printf("check: %d\n", data_check);
                return true;
        }

        return false;
    }

    size_t available() {
        if (state != PARSE_HEADER) //Only allow getting data when not actively parsing new data.
            return 0;

        return data_buf_head;
    }

    size_t read(uint8_t *dest, size_t size) {
        size = min(size, data_buf_head);

        memcpy(dest, data_buf, size);
        data_buf_head -= size;
        return size;
    }

    PacketHeader header() {
        return packet_header;
    }

    void generate_header(uint8_t *dest, uint16_t data_size, uint8_t dest_address, bool response_required) {
        dest[0] = MAGIC_START;
        dest[INDEX_ADDRESS] = (dest_address & 0x7F) | (response_required ? 0x80 : 0x00);
        dest[INDEX_SIZE_LSB] = data_size;
        dest[INDEX_SIZE_MSB] = data_size >> 8;
    }

    bool write(uint8_t *src, size_t size, uint8_t dest_address, bool response_required) {
        // Only addresses <= 0x7F valid. 0x7F is a global message
        if (address > 0x7F) return false;

        if (locked && !(address == 0x7F && response_required && packet_header.address != address))
            return false; //Only allow responses addressed to this device on a locked bus.

        uint8_t header[HEADER_SIZE];
        uint16_t checksum = 0xABAB;

        generate_header(header, size, address, response_required);

        locked = response_required;

        serial.write(header, HEADER_SIZE);
        serial.write(src, size);
        serial.write((uint8_t *) &checksum, 2);

        return true;
    }

    bool write_global(uint8_t *src, size_t size) {
        return write(src, size, 0x7F, false);
    }

    bool respond(uint8_t *src, size_t size) {
        if (!packet_header.response_expected) return false;

        // A response is a
        write(src, size, 0x7F, true);
        return true;
    }
};

#endif //DAQV7_BUSADAPTER_H
