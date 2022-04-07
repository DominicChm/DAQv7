//
// Created by Domo2 on 4/4/2022.
//

#ifndef DAQV7_DEVICEDRIVERBASE_H
#define DAQV7_DEVICEDRIVERBASE_H

#include "core/BusAdapter.h"

template<size_t buf_size>
class DeviceDriverBase {
    BusAdapter<buf_size> &bus;

public:
    DeviceDriverBase(BusAdapter<buf_size> &bus) : bus(bus) {

    }

    void loop() {
        if (bus->available()) {

        }
    }

    virtual void on_response_request(uint8_t cmd, uint8_t *data, size_t size) = 0;

    virtual void on_packet(uint8_t address, bool response_requested, uint8_t cmd, uint8_t *data, size_t size) = 0;

    virtual void on_addressed_packet(bool response_requested, uint8_t cmd, uint8_t *data, size_t size) = 0;
};

#endif //DAQV7_DEVICEDRIVERBASE_H
