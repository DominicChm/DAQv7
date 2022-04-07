//
// Created by Domo2 on 4/1/2022.
//

#ifndef DAQV7_RECIEVER_H
#define DAQV7_RECIEVER_H

#include "BUS.h"

template<size_t buffer_size>
class Receiver {
public:
    Receiver(Stream &serial, FlowController &controller, BusOptions *opts) :
            serial(serial),
            controller(controller),
            opts(opts) {}

    void loop() {
        if (serial.available()) fsm(serial.read());

        if (timed_out()) timeout_receive();
    }

    size_t available() {
        if (controller.is_receiving()) return 0; // Only allow getting data when not actively parsing new data.
        return data_head - data_tail;
    }

    uint8_t read() {
        uint8_t dat;
        read(&dat, 1);
        return dat;
    }

    size_t read(uint8_t *dest, size_t size) {
        size_t data_unread = data_head - data_tail;
        size = min(size, data_unread);

        memcpy(dest, &data[data_tail], size);
        data_tail += size;
        return size;
    }

    PacketHeader header() {
        return packet_header;
    }

private:
    enum State {
        IDLE,
        PARSE_HEADER,
        PARSE_PAYLOAD,
        PARSE_TRAILER, // Apparently the end of a packet is called a "trailer"... Who knew?
    };

    Stream &serial;
    FlowController &controller;
    BusOptions *opts;

    PacketHeader packet_header;
    PacketTrailer packet_trailer;
    State state;

    uint16_t packet_idx = 0;
    uint64_t parse_started_ms = 0;

    uint32_t num_oversized_packets;

    // Data buffer
    uint8_t data[buffer_size];
    uint16_t data_head;
    uint16_t data_tail; //Used for reading the FIFO cache.


    void timeout_receive() {
        init_parser(true);
        controller.receive_ended(packet_header);
    }

    void init_parser(bool reset_data = true) {
        state = IDLE;
        packet_idx = 0;
        parse_started_ms = millis();

        if (reset_data) {
            packet_header = {0, false, 0}; // Reset header info
            packet_trailer = {0};
            data_head = 0;
            data_tail = 0;
        }
    }

    void fsm(uint8_t b) {
        //Serial.write(b);
        switch (state) {
            case IDLE:
                if (b == HEADER_MAGIC_START) {
                    // Magic byte received - packet start.
                    init_parser(true);
                    controller.receive_started();
                    parse_started_ms = millis();
                    state = PARSE_HEADER;
                }
                break;

            case PARSE_HEADER:
                if (header_parse_fsm(b)) {
                    // Skip packets that are too big for our buffer.
                    if (packet_header.data_size > buffer_size) {
                        num_oversized_packets++;
                        controller.receive_ended(packet_header);
                        init_parser(true);
                    } else {
                        state = PARSE_PAYLOAD;
                    }
                }
                break;

            case PARSE_PAYLOAD:
                data[data_head++] = b;
                if (data_head >= packet_header.data_size) {
                    state = PARSE_TRAILER;
                }
                break;

            case PARSE_TRAILER:
                if (trailer_parse_fsm(b)) {
                    //TODO: CHECK AGAINST CHECKSUM!
                    controller.receive_ended(packet_header);
                    init_parser(false);
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

    bool timed_out() {
        return controller.is_receiving() && opts->parser_timeout_ms != 0 &&
               millis() - parse_started_ms > PARSER_TIMEOUT_MS;
    }

    const uint8_t *data_buffer() {
        return data;
    }
};

#endif //DAQV7_RECIEVER_H
