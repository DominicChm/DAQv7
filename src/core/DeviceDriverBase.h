//
// Created by Domo2 on 4/4/2022.
//

#ifndef DAQV7_DEVICEDRIVERBASE_H
#define DAQV7_DEVICEDRIVERBASE_H

#include "core/BusAdapter.h"

template<size_t buf_size>
class DeviceDriverBase {
protected:
    BusAdapter<buf_size> &bus;

public:
    DeviceDriverBase(BusAdapter<buf_size> &bus) : bus(bus) {

    }

    void loop() {
        if (bus->available()) {
            handle_bus_data();
        }
    }

    virtual void on_packet(uint8_t address, bool response_requested, uint8_t cmd, const uint8_t *data, size_t size) = 0;

    virtual void on_addressed_packet(bool response_requested, uint8_t cmd, const uint8_t *data, size_t size) = 0;

    virtual void on_global_packet(uint8_t cmd, const uint8_t *data, size_t size) = 0;

private:
    void handle_bus_data() {
        size_t packet_size = bus.available();
        uint8_t *packet = bus.read_data_buffer();

        uint8_t cmd = packet[0];
        uint8_t *data = packet + 1;
        size_t data_size = packet_size - 1;

        if (bus.header().address == bus.address()) {
            //Addressed here
            on_addressed_packet(bus.header().response_expected, cmd, data, data_size);
        } else if (bus.header().address == GLOBAL_ADDRESS) {
            //Global
            on_global_packet(cmd, data, data_size);
        } else {
            on_packet(bus.header().address, bus.header().response_expected, cmd, data, data_size);
        }
    }
};

#endif //DAQV7_DEVICEDRIVERBASE_H
