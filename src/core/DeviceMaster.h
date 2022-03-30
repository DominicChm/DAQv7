//
// Created by Dominic on 3/29/2022.
//

#ifndef DAQV7_DEVICEMASTER_H
#define DAQV7_DEVICEMASTER_H

#include "BusDevice.h"

class DeviceMaster {
private:
    const static size_t data_buf_size = 1;

    BusDevice &device;
    ApplicationProtocol &adapter;

    uint8_t data_buffer[data_buf_size];
public:
    DeviceMaster(BusDevice &device, ApplicationProtocol &adapter) : device(device), adapter(adapter) {

    }

    void setData(uint8_t *data, size_t size) {
        memcpy(data_buffer, data, min(size, data_buf_size));
    }

};

#endif //DAQV7_DEVICEMASTER_H
