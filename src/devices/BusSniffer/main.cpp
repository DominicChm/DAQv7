#include <Arduino.h>
#include <core/ApplicationProtocol.h>
#include "core/BusAdapter.h"
#include "core/DeviceMaster.h"

BusAdapter bus(Serial);

void parse_data(uint8_t *data, size_t size) {
    uint8_t cmd = data[0];
//    Serial.printf("ADDR: %d; RES: %d; SIZE: %d; CMD: %d\r\n",
//                  bus.header().address,
//                  bus.header().response_expected,
//                  (uint16_t) bus.header().data_size,
//                  cmd);

    if (bus.header().address != 0x55) return;


    switch (cmd) {
        case 0x00: //HEARTBEAT
            //Serial.print("HEARTBEAT");
            bus.respond(nullptr, 0);
            break;

    }
}

void setup() {
    Serial.begin(115200);
}


uint64_t last_print = 0;

void loop() {
    bus.loop();
    if (bus.available()) {
        uint8_t data[255];
        parse_data(data, bus.read(data, 255));
    }
}

