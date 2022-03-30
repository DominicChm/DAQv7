#include <Arduino.h>
#include <core/ApplicationProtocol.h>
#include "core/BusAdapter.h"
#include "core/DeviceMaster.h"
#include "device.h"

BusAdapter bus(Serial);
//DeviceMaster dev(device, bus);

void setup() {
    Serial.begin(115200);

// write your initialization code here
}

uint64_t last_print = 0;

void loop() {
    bus.loop();
    if (millis() - last_print > 1000) {
        last_print = millis();
    }
}