#pragma once
/**
 * ╔══════════════════════════════════════════════════════════════════════════╗
 *  ui.h  —  Screen State Machine + Button Handler  v5.0
 *
 *  BUTTON BEHAVIOUR
 *  ─────────────────
 *  HOME    short → MENU
 *  HOME    long  → (no-op)
 *  MENU    short → cycle items
 *  MENU    long  → HOME
 *  CHANT   short → forwarded to chantUpdate()
 *  CHANT   long  → MENU
 *  STREAK  short → MENU
 *  STREAK  long  → HOME
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

// ── Button pin ──────────────────────────────────────────────────
#define BTN_PIN  18   // GPIO0 = built-in BOOT button. Change as needed.
                     // Wiring: GPIO → button → GND (active-LOW)

// ── Button tuning ───────────────────────────────────────────────
static const uint32_t BTN_DEBOUNCE_MS  =  40;
static const uint32_t BTN_LONGPRESS_MS = 600;

// ── Screen states ───────────────────────────────────────────────
enum ScreenState : uint8_t {
    SCREEN_HOME   = 0,
    SCREEN_MENU   = 1,
    SCREEN_CHANT  = 2,
    SCREEN_STREAK = 3
};

// ── Menu ────────────────────────────────────────────────────────
static const uint8_t MENU_ITEM_COUNT = 3;

static const char * const MENU_LABELS[MENU_ITEM_COUNT] = {
    "Home", "Chant", "Streak"
};

static const ScreenState MENU_TARGETS[MENU_ITEM_COUNT] = {
    SCREEN_HOME, SCREEN_CHANT, SCREEN_STREAK
};

// ── Button event flags (frame-scoped, cleared each uiUpdate()) ──
//    Read these in loop() to forward events to sub-screens.
extern bool buttonShortPress;
extern bool buttonLongPress;

// ── API ─────────────────────────────────────────────────────────
void        uiInit();
ScreenState uiUpdate();
ScreenState uiCurrentScreen();
void        renderMenu();
void        renderPlaceholder(const char *title);