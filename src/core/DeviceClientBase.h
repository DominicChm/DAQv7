//
// Created by Dominic on 3/30/2022.
//

#ifndef DAQV7_DEVICECLIENTBASE_H
#define DAQV7_DEVICECLIENTBASE_H

#include <Arduino.h>

/**
 * Base class to expose device APIs to the bus master
 */

class DeviceClientBase {
protected:
    uint8_t address;
    BusAdapter &adapter;
    bool pending = false;

public:
    DeviceClientBase(uint8_t address, BusAdapter &adapter) : address(address), adapter(adapter) {

    }

    void heartbeat() {
        pending = true;
        uint8_t dat[] = {0x00};
        adapter.write(dat, 1, address, true);
    }

    bool is_pending() {
        return pending;
    }

    void loop() {

    }
};

#endif //DAQV7_DEVICECLIENTBASE_H
