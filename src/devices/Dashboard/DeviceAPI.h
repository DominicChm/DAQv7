//
// Created by Dominic on 3/30/2022.
//

#ifndef DAQV7_DASHBOARD_DEVICECLIENT_H
#define DAQV7_DASHBOARD_DEVICECLIENT_H

#include <core/DeviceClientBase.h>

namespace Dashboard {
    class DeviceClient : public DeviceClientBase {
    public:
        explicit DeviceClient(uint8_t address, BusAdapter &adapter) : DeviceClientBase(address, adapter) {

        }
    };
}


#endif //DAQV7_DEVICECLIENTBASE_H
