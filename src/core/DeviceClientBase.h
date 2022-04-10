//
// Created by Domo2 on 4/4/2022.
//

#ifndef DAQV7_DEVICECLIENTBASE_H
#define DAQV7_DEVICECLIENTBASE_H

#include "BusAdapter.h"

class DeviceClientBase {
public:
    DeviceClientBase(BusAdapter *bus, bool is_master) : bus(bus), _is_master(is_master) {}

    void loop() {
//        bus->loop();
//        if (bus.available()) {
//
//        }
    }

    // Called when a new packet is available on the bus.
    void update(PacketHeader p, uint8_t *data, size_t size) {

    }

    // Called to generate a response from an implementing class

    bool is_master() {
        return _is_master;
    }

protected:
    BusAdapter *bus;
    bool _is_master;

};

#endif //DAQV7_DEVICECLIENTBASE_H
