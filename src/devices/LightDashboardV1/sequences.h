//
// Created by Dominic on 4/10/2022.
//

#ifndef DAQV7_SEQUENCES_H
#define DAQV7_SEQUENCES_H

#include <jled.h>
#include "pins.h"

namespace Sequences {
    const int chase_delay = 100;
    JLed chase[] = {
            JLed(PIN_LED_0_G).Blink(chase_delay, 1),
            JLed(PIN_LED_1_G).Blink(chase_delay, 1),
            JLed(PIN_LED_2_G).Blink(chase_delay, 1),
            JLed(PIN_LED_3_G).Blink(chase_delay, 1),
            JLed(PIN_LED_4_G).Blink(chase_delay, 1),
            JLed(PIN_LED_5_G).Blink(chase_delay, 1),
            JLed(PIN_LED_5_R).Blink(chase_delay, 1),
            JLed(PIN_LED_4_R).Blink(chase_delay, 1),
            JLed(PIN_LED_3_R).Blink(chase_delay, 1),
            JLed(PIN_LED_2_R).Blink(chase_delay, 1),
            JLed(PIN_LED_1_R).Blink(chase_delay, 1),
            JLed(PIN_LED_0_R).Blink(chase_delay, 1),
    };

    const int blink_on = 500;
    const int blink_off = 500;
    JLed blink[] = {
            JLed(PIN_LED_0_G).Blink(blink_on, blink_off),
            JLed(PIN_LED_1_G).Blink(blink_on, blink_off),
            JLed(PIN_LED_2_G).Blink(blink_on, blink_off),
            JLed(PIN_LED_3_G).Blink(blink_on, blink_off),
            JLed(PIN_LED_4_G).Blink(blink_on, blink_off),
            JLed(PIN_LED_5_G).Blink(blink_on, blink_off),
            JLed(PIN_LED_5_R).Blink(blink_on, blink_off),
            JLed(PIN_LED_4_R).Blink(blink_on, blink_off),
            JLed(PIN_LED_3_R).Blink(blink_on, blink_off),
            JLed(PIN_LED_2_R).Blink(blink_on, blink_off),
            JLed(PIN_LED_1_R).Blink(blink_on, blink_off),
            JLed(PIN_LED_0_R).Blink(blink_on, blink_off),
    };
}
#endif //DAQV7_SEQUENCES_H
