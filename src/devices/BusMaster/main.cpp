#include <Arduino.h>
#include <core/BusAdapter.h>
#include <devices/Dashboard/DeviceAPI.h>

BusAdapter bus(Serial, 0x01);

/* Devices */
Dashboard::DeviceClient driver_dash(1, bus);

/* ARRAY OF ALL DEVICES */
DeviceClientBase *devices[] = {
        &driver_dash,
};
size_t num_devices = sizeof(devices) / sizeof(devices[0]);

void setup() {
    Serial.begin(115200);
    //Heartbeat all sensors
    for (size_t i = 0; i < num_devices; i++) {
        devices[i]->
    }
}

void loop() {
// write your code here
}