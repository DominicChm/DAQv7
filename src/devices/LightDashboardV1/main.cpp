//
// Created by Dominic on 4/10/2022.
//


#include <jled.h>
#include "sequences.h"
#include "pins.h"


void setup() {
    auto startup = JLedSequence(JLedSequence::eMode::SEQUENCE, Sequences::chase).Repeat(2);
    while (startup.Update());

    startup = JLedSequence(JLedSequence::eMode::PARALLEL, Sequences::blink).Repeat(2);
    while (startup.Update());

}

void loop() {
}