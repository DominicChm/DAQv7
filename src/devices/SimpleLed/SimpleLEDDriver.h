//
// Created by Domo2 on 4/6/2022.
//

#ifndef DAQV7_SIMPLELEDMASTER_H
#define DAQV7_SIMPLELEDMASTER_H

#include <core/DeviceDriverBase.h>

template<size_t buffer_size>
class SimpleLEDMaster : public DeviceDriverBase<buffer_size> {
    explicit SimpleLEDMaster(BusAdapter<buffer_size> &bus) : DeviceDriverBase<buffer_size>(bus) {

    }

    virtual void on_response_request(uint8_t cmd, uint8_t *data, size_t size) = 0;

    virtual void on_packet(uint8_t address, bool response_requested, uint8_t cmd, uint8_t *data, size_t size) = 0;

    virtual void on_addressed_packet(bool response_requested, uint8_t cmd, uint8_t *data, size_t size) = 0;
};

#endif //DAQV7_SIMPLELEDMASTER_H
