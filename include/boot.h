#pragma once
/**
 * ╔══════════════════════════════════════════════════════════════════════════╗
 *  boot.h  —  Boot Sequence  v6.0
 *
 *  FLOW:
 *  BOOT_GREETING   "Happy B'day Dhanu"  1.5 s  — LANDSCAPE rotation(0)
 *  BOOT_SIGNATURE  "By Leen"            1.2 s  — LANDSCAPE rotation(0)
 *  BOOT_BUTTERFLY  butterfly            3.0 s  — PORTRAIT  rotation(1) restored
 *  BOOT_DONE       → SCREEN_HOME
 *  BOOT_BUTTERFLY  butterfly    3.0 s
 *  BOOT_DONE       → SCREEN_HOME
 * ╚══════════════════════════════════════════════════════════════════════════╝
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

static const uint32_t BOOT_GREETING_MS  = 1500;   // birthday text gets 300ms extra
static const uint32_t BOOT_SIGNATURE_MS = 1200;
static const uint32_t BOOT_BUTTERFLY_MS = 3000;

static const float BFLY_SWAY_AMP    = 5.0f;
static const float BFLY_SWAY_PERIOD = 1.0f;
static const float BFLY_WING_PERIOD = 0.25f;

enum BootState : uint8_t {
    BOOT_GREETING  = 0,
    BOOT_SIGNATURE = 1,
    BOOT_BUTTERFLY = 2,
    BOOT_DONE      = 3
};

void bootInit();
bool bootUpdate(float dt);
void renderBoot();    