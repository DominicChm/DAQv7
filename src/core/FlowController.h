//
// Created by Domo2 on 4/1/2022.
//

#ifndef DAQV7_FLOWCONTROLLER_H
#define DAQV7_FLOWCONTROLLER_H

#include "BUS.h"
#include "BUS.h"

class FlowController {
    bool bus_locked = false;
    bool receiving = false;
    bool transmitting = false;
    PacketHeader last_header;

    uint64_t lock_started_ms = 0;

    BusOptions *opts;


    void ingest_header(PacketHeader header) {
        last_header = header;
        bus_locked = header.response_expected && header.address != GLOBAL_ADDRESS;
        lock_started_ms = millis();
    }

public:
    FlowController(BusOptions *opts) : opts(opts) {}

    void loop() {
        if (lock_timed_out()) bus_locked = false;
    }

    bool lock_timed_out() {
        return bus_locked && opts->lock_timeout_ms != 0 && (millis() - lock_started_ms > BUS_TIMEOUT_MS);
    }

    void receive_started() {
        bus_locked = true;
        receiving = true;
        transmitting = false; //We should NEVER be txing and also receiving. If we are, something is terribly wrong.

        lock_started_ms = millis();
    }

    void receive_ended(PacketHeader header) {
        receiving = false;
        ingest_header(header);
    }

    bool begin_response() {
        if (!bus_locked || last_header.address != opts->address || receiving || transmitting) return false;
        transmitting = true;
        return true;
    }

    bool begin_transmit(uint8_t address) {
        if (bus_locked || receiving || transmitting) return false;
        transmitting = true;
        return true;
    }

    void end_transmit(PacketHeader header) {
        ingest_header(header);
        transmitting = false;
    }

    bool is_bus_locked() {
        return bus_locked;
    }

    // Whether the bus is currently in use.
    // Note: A locked bus isn't necessarily busy.
    bool bus_busy() {
        return transmitting || receiving;
    }

    bool is_transmitting() {
        return transmitting;
    }

    bool is_receiving() {
        return receiving;
    }
};

#endif //DAQV7_FLOWCONTROLLER_H
