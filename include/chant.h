#pragma once
/**
 * chant.h  —  Japa Counter Screen  v6.0
 */

#include <Arduino.h>
#include <Adafruit_SSD1306.h>

extern Adafruit_SSD1306 oled;

#ifndef CANVAS_W
  #define CANVAS_W  64
#endif
#ifndef CANVAS_H
  #define CANVAS_H 128
#endif

static const uint16_t CHANT_TARGET = 108;

enum ChantState : uint8_t {
    CHANT_IDLE     = 0,
    CHANT_COUNTING = 1,
    CHANT_COMPLETE = 2
};

extern ChantState chantState;
extern uint16_t   chantCount;

void chantEnter();
void chantUpdate(bool shortPress, bool longPress);
void renderChant();