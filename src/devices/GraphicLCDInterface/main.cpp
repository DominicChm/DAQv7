#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>
#include "CPRLogo.h"
#include "core/BusAdapter.h"

#define PIN_BTN 9
#define PIN_ENC1 8
#define PIN_ENC2 7

BusOptions opts{0x02, 6, 9600, 0, 0};
BusAdapter<0> bus(Serial, &opts);
U8G2_ST7920_128X64_1_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* CS=*/ 10, /* reset=*/ 8);

uint8_t btn_mask(uint8_t pin) {
    return digitalRead(pin) ? 0xFF : 0x00;
}

void draw(void) {
    u8g2.drawXBMP(0, 0, 128, 23, CPR_logo);
    u8g2.setFont(u8g2_font_6x10_mf);
    uint16_t pos = 20;
    u8g2.drawButtonUTF8(3, pos += 12, U8G2_BTN_BW1 | (U8G2_BTN_INV & btn_mask(PIN_BTN)), 100, 1, 1, "Button");
    u8g2.drawButtonUTF8(3, pos += 12, U8G2_BTN_BW1 | (U8G2_BTN_INV & btn_mask(PIN_ENC1)), 100, 1, 1, "Enc1");
    u8g2.drawButtonUTF8(3, pos += 12, U8G2_BTN_BW1 | (U8G2_BTN_INV & btn_mask(PIN_ENC2)), 100, 1, 1, "Enc2");
}


void setup() {
    pinMode(PIN_BTN, INPUT_PULLUP);
    pinMode(PIN_ENC1, INPUT_PULLUP);
    pinMode(PIN_ENC2, INPUT_PULLUP);
    // put your setup code here, to run once:
    u8g2.begin();
    u8g2.clearBuffer();

}

void loop() {
    // put your main code here, to run repeatedly:
    u8g2.firstPage();
    do {
        draw();
    } while (u8g2.nextPage());

    //delay(1000);

}