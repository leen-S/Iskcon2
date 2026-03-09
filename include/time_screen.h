#pragma once
/**
 * ╔══════════════════════════════════════════════════════════════════════════╗
 *  time_screen.h  —  DS3231 RTC Time Applet  v10.0
 *
 *  DISPLAY:  LANDSCAPE  (setRotation(0) → 128 px wide × 64 px tall)
 *  LAYOUT:
 *    y ≈  6   "Time"          TextSize 1  — centred label
 *    y ≈ 20   "HH:MM:SS"      TextSize 2  — large clock
 *    y ≈ 50   "9th March 2026" TextSize 1  — human date
 *
 *  NAVIGATION:
 *    Long press  → SCREEN_MENU  (rotation restored to portrait in ui.cpp)
 *    Short press → no-op
 *
 *  WIRING:
 *    DS3231 SDA → GPIO 21
 *    DS3231 SCL → GPIO 22
 *    VCC → 3.3V    GND → GND
 *
 *  DEPENDENCY:
 *    RTClib by Adafruit  (add to platformio.ini lib_deps)
 * ╚══════════════════════════════════════════════════════════════════════════╝
 */

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <RTClib.h>

extern Adafruit_SSD1306 oled;
extern RTC_DS3231        rtc;

// Landscape canvas dimensions (opposite of portrait)
#define LS_CANVAS_W  128
#define LS_CANVAS_H   64

// ── API ─────────────────────────────────────────────────────────
bool timeScreenInit();   // call in setup() — returns false if RTC missing
void renderTimeScreen(); // draw frame into buffer — no oled.display()

// ── Helpers (exposed for unit testing convenience) ───────────────
const char *getDaySuffix(uint8_t day);
const char *getMonthName(uint8_t month);