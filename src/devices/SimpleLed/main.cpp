#include <Arduino.h>
#include "core/BusAdapter.h"
#include "SimpleLEDDriver.h"

BusOptions opts{0x01, D1, 9600, 0, 0};
BusAdapter<128> bus(Serial, &opts);
SimpleLEDDriver<128> mas(bus);

void parse_data(uint8_t *data, size_t size) {
    uint8_t cmd = data[0];
//    Serial.printf("ADDR: %d; RES: %d; SIZE: %d; CMD: %d\r\n",
//                  bus.header().address,
//                  bus.header().response_expected,
//                  (uint16_t) bus.header().data_size,
//                  cmd);
    //Serial.println(cmd);

    if (bus.header().address != bus.address()) return;


    switch (cmd) {
        case 0x00: //HEARTBEAT
            //Serial.print("HEARTBEAT");
            bus.respond(nullptr, 0);
            break;

        case 0x01: //ON
            digitalWrite(LED_BUILTIN, LOW);
            break;

        case 0x02: //OFF
            digitalWrite(LED_BUILTIN, HIGH);
            break;

    }
}

void setup() {
    Serial1.begin(9600);
    Serial.begin(9600);
    bus.begin();
    Serial.println("start");

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
    digitalWrite(LED_BUILTIN, HIGH);
}


uint64_t last_print = 0;

void loop() {
    if (Serial1.available()) {
        //Serial.println(Serial1.peek());
    }
    bus.loop();
    if (bus.available()) {
        uint8_t data[255];
        parse_data(data, bus.read(data, 255));
    }

//    if(millis() - last_print > 1000) {
//        Serial.println(bus.parseState);
//        last_print = millis();
//    }
}

