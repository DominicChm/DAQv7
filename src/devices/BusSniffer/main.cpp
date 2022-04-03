#include <Arduino.h>
#include "core/BusAdapter.h"

BusAdapter bus(Serial, {0x00, D1, 9600, 0, 0});

void parse_data(uint8_t *data, size_t size) {
    uint8_t cmd = data[0];
//    Serial.printf("ADDR: %d; RES: %d; SIZE: %d; CMD: %d\r\n",
//                  bus.header().address,
//                  bus.header().response_expected,
//                  (uint16_t) bus.header().data_size,
//                  cmd);

    if (bus.header().address != bus.address()) return;


    switch (cmd) {
        case 0x00: //HEARTBEAT
            //Serial.print("HEARTBEAT");
            bus.respond(nullptr, 0);
            break;

    }
}

void setup() {
    Serial.begin(9600);
    bus.begin();
}


uint64_t last_print = 0;
bool ledState = false;

void loop() {
    bus.loop();
    if (bus.available()) {
        uint8_t data[255];
        parse_data(data, bus.read(data, 255));
    }

    if (millis() - last_print > 1000) {
        //Serial.println(bus.parseState);
        if (ledState) {
            uint8_t dat = 0x01;
            bus.write(&dat, 1, 0x01, false);
        } else {
            uint8_t dat = 0x02;
            bus.write(&dat, 1, 0x01, false);
        }

        ledState = !ledState;
        last_print = millis();
    }
}

