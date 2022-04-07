//
// Created by Domo2 on 4/4/2022.
//

#ifndef DAQV7_DEVICECLIENTBASE_H
#define DAQV7_DEVICECLIENTBASE_H

class DeviceClientBase {
public:
    DeviceClientBase(BusAdapter *bus) : bus(bus) {}

    void loop() {
//        bus->loop();
//        if (bus.available()) {
//
//        }
    }

    // Called to generate a response from an implementing class


private:
    BusAdapter *bus;
};

#endif //DAQV7_DEVICECLIENTBASE_H
