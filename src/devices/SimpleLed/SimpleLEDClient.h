//
// Created by Domo2 on 4/8/2022.
//

#ifndef DAQV7_SIMPLELEDCLIENT_H
#define DAQV7_SIMPLELEDCLIENT_H

#include "core/DeviceClientBase.h"

class SimpleLEDClient : public DeviceClientBase {
    SimpleLEDClient(BusAdapter &bus) : DeviceClientBase(bus) {

    }

    void on() {
        if (!_is_master) return;
        this->bus.
    }
};

#endif //DAQV7_SIMPLELEDCLIENT_H
