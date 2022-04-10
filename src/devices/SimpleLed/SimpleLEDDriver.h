//
// Created by Domo2 on 4/6/2022.
//

#ifndef DAQV7_SIMPLELEDMASTER_H
#define DAQV7_SIMPLELEDMASTER_H

#include <core/DeviceDriverBase.h>

template<size_t buffer_size>
class SimpleLEDDriver : public DeviceDriverBase<buffer_size> {
public:
    explicit SimpleLEDDriver(BusAdapter<buffer_size> &bus) : DeviceDriverBase<buffer_size>(bus) {

    }

    bool led_state() {
        return led_state;
    }

private:
    bool _led_state = false;

    virtual void on_packet(uint8_t address, bool response_requested, uint8_t cmd, const uint8_t *data, size_t size) {
        // Called with bus packets that weren't addressed to us.
    }

    virtual void on_addressed_packet(bool response_requested, uint8_t cmd, const uint8_t *data, size_t size) {
        uint8_t *resp;
        switch (cmd) {
            case 0x00: //TODO: IMPLEMENT IN DEVICE BASE
                resp = {};
                this->bus.respond(resp, 0);

            case 0x01: //LED ON
                _led_state = false;

            case 0x02: //LED OFF
                _led_state = true;
        }
    }

    virtual void on_global_packet(uint8_t cmd, const uint8_t *data, size_t size) {
    }

};

#endif //DAQV7_SIMPLELEDMASTER_H
