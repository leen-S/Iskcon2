#pragma once
/**
 * ╔══════════════════════════════════════════════════════════════════════════╗
 *  ui.h  —  Screen State Machine + Button Handler  v10.0
 *
 *  v10.0 ADDITIONS
 *  ────────────────
 *  + SCREEN_TIME state  — landscape RTC clock applet
 *  + Menu expanded to 4 items: Home / Chant / Streak / Time
 *  + Rotation managed here: setRotation(0) on TIME entry,
 *    setRotation(1) on TIME exit — no other file changes rotation.
 *
 *  BUTTON BEHAVIOUR
 *  ─────────────────
 *  BOOT    short/long → skip to HOME
 *  HOME    short      → open MENU
 *  MENU    short      → cycle items
 *  MENU    long       → select highlighted item
 *  CHANT   short      → forwarded via buttonShortPress to chantUpdate()
 *  CHANT   long       → return to MENU
 *  STREAK  short      → return to MENU
 *  STREAK  long       → return to HOME
 *  TIME    short      → no-op
 *  TIME    long       → return to MENU  (restores portrait rotation)
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
#define BTN_PIN  4    // GPIO0 — active-LOW, internal pull-up
                      // Wiring: GPIO0 → button → GND

// ── Button timing ───────────────────────────────────────────────
static const uint32_t BTN_DEBOUNCE_MS  =  40;
static const uint32_t BTN_LONGPRESS_MS = 800;

// ═══════════════════════════════════════════════════════════════
//  SCREEN STATE MACHINE
// ═══════════════════════════════════════════════════════════════
enum ScreenState : uint8_t {
    SCREEN_BOOT   = 0,
    SCREEN_HOME   = 1,
    SCREEN_MENU   = 2,
    SCREEN_CHANT  = 3,
    SCREEN_STREAK = 4,
    SCREEN_TIME   = 5    // landscape RTC clock — rotation(0) while active
};

// ── Menu ────────────────────────────────────────────────────────
static const uint8_t MENU_ITEM_COUNT = 4;

static const char * const MENU_LABELS[MENU_ITEM_COUNT] = {
    "Home", "Chant", "Streak", "Time"
};

static const ScreenState MENU_TARGETS[MENU_ITEM_COUNT] = {
    SCREEN_HOME, SCREEN_CHANT, SCREEN_STREAK, SCREEN_TIME
};

// ── Frame-scoped button event flags ─────────────────────────────
extern bool buttonShortPress;
extern bool buttonLongPress;

// ── API ─────────────────────────────────────────────────────────
void        uiInit();
ScreenState uiUpdate();
ScreenState uiCurrentScreen();
void        uiSetScreen(ScreenState s);
void        renderMenu();
void        renderPlaceholder(const char *title);