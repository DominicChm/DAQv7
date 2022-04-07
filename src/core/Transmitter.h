//
// Created by Domo2 on 4/1/2022.
//

#ifndef DAQV7_TRANSMITTER_H
#define DAQV7_TRANSMITTER_H

template<size_t buffer_size>
class Transmitter {
public:
    Transmitter(Stream &serial, FlowController &controller, BusOptions *opts) :
            serial(serial),
            controller(controller),
            opts(opts) {
        pinMode(opts->pin_driver_enable, OUTPUT);
        digitalWrite(opts->pin_driver_enable, LOW);
        transmission_finish_delay = 1000000 * 8 / opts->baudrate; //BAUDRATE
    }

    bool begin() {
        hw_tx_buffer_size = serial.availableForWrite();
        return true;
    }

    void loop() {
        fsm();

        // Check and handle timeouts
    }


    // Writes a packet to the bus. Non-blocking writes are performed by default.
    // Byte-by-byte transmission (manual metering) is HANDLED BY THE CLASS!
    bool write(uint8_t *payload, size_t size, uint8_t dest_address, bool response_required) {
        if (!controller.begin_transmit(dest_address)) return false;

        return write_bus(payload, size, dest_address, response_required);
    }

    bool write_global(uint8_t *src, size_t size) {
        return write(src, size, GLOBAL_ADDRESS, false);
    }

    bool respond(uint8_t *src, size_t size) {
        if (!controller.begin_response()) return false;

        // A response is to the global address, with the response bit set.
        return write_bus(src, size, GLOBAL_ADDRESS, true);
    }

private:
    enum State {
        IDLE,
        BUFFERING,
        TRANSMITTING,
        DELAY_DISABLE,
    };

    uint64_t transmission_finished_us = 0;
    uint64_t transmission_finish_delay = 0;

    State state = IDLE;

    // Set at construction time as the number of free bytes in the serial tx buffer.
    // Manually writing to the bus before initializing the adapter will cause this to be off!!
    int hw_tx_buffer_size = 0;

    Stream &serial;
    FlowController &controller;
    BusOptions *opts;

    // Header FSM vars
    PacketHeader transmitted_header;

    // TX buffer
    uint8_t transmit_size = 0;
    uint8_t transmit_idx = 0;
    uint8_t transmit_buf[buffer_size];

    void fsm() {
        switch (state) {
            case IDLE:
                // Wait for some data to be put into our buffer.
                if (transmit_idx < transmit_size) { // A wild Unwritten Data has appeared!
                    digitalWrite(opts->pin_driver_enable, HIGH); // Enable the bus drivers
                    state = BUFFERING;
                }
                break;


            case BUFFERING:
                if(!controller.is_transmitting()) state = IDLE;

                // Buffer the data in our buffer into the HW serial TX buffer
                if (hw_tx_buffer_size > 0) {
                    // We have a non-zero tx buffer. Fill it.
                    while (serial.availableForWrite() > 0 && transmit_idx < transmit_size)
                        serial.write(transmit_buf[transmit_idx++]);

                } else {
                    // There's no hw buffer. Fall back to 1-by-1 writes.
                    serial.write(transmit_buf[transmit_idx++]);
                }

                // We've buffered all our data.
                if (transmit_idx >= transmit_size)
                    state = TRANSMITTING;

                break;


            case TRANSMITTING:
                if(!controller.is_transmitting()) state = IDLE;

                // Wait until the hardware TX buffer is empty (available = size)
                if (hw_tx_buffer_size - serial.availableForWrite() <= 0 ) {
                    transmission_finished_us = micros();
                    state = DELAY_DISABLE;
                }
                break;


            case DELAY_DISABLE:
                if(!controller.is_transmitting()) state = IDLE;
                // Delay disabling RS-485 drivers for one extra byte.
                // The HW tx buffer "looks" empty while the last byte is transmitting.
                // Disabling early chops the last byte off the packet, which is bad :)
                if (micros() - transmission_finished_us > transmission_finish_delay) {
                    controller.end_transmit(transmitted_header);
                    digitalWrite(opts->pin_driver_enable, LOW);
                    state = IDLE;
                }
                break;
        }
    }


    bool write_bus(uint8_t *payload, size_t size, uint8_t dest_address, bool response_required) {
        if (!controller.is_transmitting()) return false; //How did we get to this state???
        // Only addresses <= 0x7F valid. 0x7F is a global message

        transmit_size = 0;
        transmit_idx = 0;

        transmitted_header = generate_header(transmit_buf, size, dest_address, response_required);
        transmit_size += HEADER_SIZE;

        memcpy(&(transmit_buf[transmit_size]), payload, size);
        transmit_size += size;

        generate_trailer(&(transmit_buf[transmit_size]), payload, size);
        transmit_size += TRAILER_SIZE;

        return true;
    }
};

#endif //DAQV7_TRANSMITTER_H
