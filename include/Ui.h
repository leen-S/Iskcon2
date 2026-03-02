#pragma once
/**
 * ╔══════════════════════════════════════════════════════════════════════════╗
 *  ui.h  —  Screen State Machine + Button Handler  v9.0
 *
 *  Reverted from v8.0: all deep sleep / SCREEN_OFF / esp_sleep removed.
 *  Always-on firmware. Button on GPIO0, active-LOW, INPUT_PULLUP.
 *
 *  BUTTON BEHAVIOUR
 *  ─────────────────
 *  BOOT    short/long → skip to HOME
 *  HOME    short      → open MENU
 *  MENU    short      → cycle items
 *  MENU    long       → select highlighted item
 *  CHANT   short      → forwarded to chantUpdate() via buttonShortPress
 *  CHANT   long       → return to MENU
 *  STREAK  short      → return to MENU
 *  STREAK  long       → return to HOME
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
#define BTN_PIN  0    // GPIO0 — built-in BOOT button on most DevKits
                      // Wiring: GPIO0 → button → GND  (active-LOW)
                      // Internal pull-up via pinMode(BTN_PIN, INPUT_PULLUP)

// ── Button timing ───────────────────────────────────────────────
static const uint32_t BTN_DEBOUNCE_MS  =  40;   // ms — noise filter
static const uint32_t BTN_LONGPRESS_MS = 800;   // ms — long press threshold

// ═══════════════════════════════════════════════════════════════
//  SCREEN STATE MACHINE
// ═══════════════════════════════════════════════════════════════
enum ScreenState : uint8_t {
    SCREEN_BOOT   = 0,
    SCREEN_HOME   = 1,
    SCREEN_MENU   = 2,
    SCREEN_CHANT  = 3,
    SCREEN_STREAK = 4
};

// ── Menu ────────────────────────────────────────────────────────
static const uint8_t MENU_ITEM_COUNT = 3;

static const char * const MENU_LABELS[MENU_ITEM_COUNT] = {
    "Home", "Chant", "Streak"
};

static const ScreenState MENU_TARGETS[MENU_ITEM_COUNT] = {
    SCREEN_HOME, SCREEN_CHANT, SCREEN_STREAK
};

// ── Frame-scoped button event flags ─────────────────────────────
// Cleared at the top of uiUpdate() every frame.
// Read in loop() to forward events to sub-screens (e.g. chantUpdate).
extern bool buttonShortPress;
extern bool buttonLongPress;

// ── API ─────────────────────────────────────────────────────────
void        uiInit();
ScreenState uiUpdate();
ScreenState uiCurrentScreen();
void        uiSetScreen(ScreenState s);
void        renderMenu();
void        renderPlaceholder(const char *title);