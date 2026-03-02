#pragma once
/**
 * leaf_breeze.h  —  Vrindavan Breeze background layer  v6.0
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

#ifndef TWO_PI
  #define TWO_PI 6.28318530718f
#endif

#define LEAF_COUNT  4

static const float LEAF_PERIOD     = 3.0f;
static const float LEAF_SWAY_AMP   = 3.5f;
static const float LEAF_FALL_SPEED = 10.0f;

static const uint8_t LEAF_SHAPE_A[3] = { 0x40, 0xE0, 0x40 };
static const uint8_t LEAF_SHAPE_B[3] = { 0x60, 0xC0, 0x40 };
static const uint8_t LEAF_SHAPE_C[3] = { 0xC0, 0xE0, 0x80 };

static const uint8_t * const LEAF_SHAPES[3] = {
    LEAF_SHAPE_A, LEAF_SHAPE_B, LEAF_SHAPE_C
};

struct Leaf {
    float   yPos;
    float   xBase;
    float   phase;
    float   swayPhase;
    uint8_t shapeIdx;
};

void initLeaves();
void updateLeaves(float dt);
void renderLeaves();