//
// Created by Dominic on 3/29/2022.
//

#ifndef DAQV7_BUSADAPTER_H
#define DAQV7_BUSADAPTER_H

#include <Arduino.h>
#include "FlowController.h"
#include "Transmitter.h"
#include "Reciever.h"

/**
 * Handles parsing and encoding of packets on the bus.
 * Also handles control flow.
 */
template<size_t buffer_size = 256>
class BusAdapter {
    BusOptions *opts;

    FlowController controller;
    Receiver<buffer_size> receiver;
    Transmitter<buffer_size> transmitter;
public:
    BusAdapter(Stream &serial, BusOptions *opts) :
            opts(opts),
            controller(FlowController(opts)),
            receiver(Receiver<buffer_size>(serial, controller, opts)),
            transmitter(Transmitter<buffer_size>(serial, controller, opts)) {

    }

    BusAdapter(HardwareSerial &serial, BusOptions *opts) : BusAdapter((Stream &) serial, opts) {
        Serial.begin(opts->baudrate);
    }

    void loop() {
        transmitter.loop();
        receiver.loop();
        controller.loop();
    }

    bool begin() {
        return transmitter.begin();
    }

    size_t available() {
        return receiver.available();
    }

    uint8_t read() { //TODO: TEST
        return receiver.read();
    }

    size_t read(uint8_t *dest, size_t size) { //TODO: TEST
        return receiver.read(dest, size);
    }

    bool write(uint8_t *payload, size_t size, uint8_t dest_address, bool response_required) {
        return transmitter.write(payload, size, dest_address, response_required);
    }

    bool write_global(uint8_t *src, size_t size) {
        return transmitter.write_global(src, size);
    }

    bool respond(uint8_t *src, size_t size) {
        return transmitter.respond(src, size);
    }

    PacketHeader header() {
        return receiver.header();
    }

    uint8_t address() {
        return opts->address;
    }

    const uint8_t *data_buffer() {
        return receiver.data_buffer();
    }
};

#endif //DAQV7_BUSADAPTER_H
