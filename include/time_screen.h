#pragma once
/**
 * ╔══════════════════════════════════════════════════════════════════════════╗
 *  time_screen.h  —  DS3231 RTC Time Applet  v11.0
 *
 *  v11.0 FIX: rtc.adjust() is ONLY called when rtc.lostPower() returns true.
 *  Normal reboot / upload no longer resets the clock.
 *
 *  I2C SHARING NOTE
 *  ─────────────────
 *  Wire.begin(21, 22) is called once in main setup() before both
 *  oled.begin() and rtc.begin() — both devices share the same bus safely
 *  because RTClib and Adafruit SSD1306 both use the same Wire instance
 *  and take turns on the bus per transaction.
 *
 *  DISPLAY:  LANDSCAPE  setRotation(0) → 128 px wide × 64 px tall
 *  LAYOUT:
 *    y ≈  6   "Time"            TextSize 1  centred label
 *    y ≈ 22   "HH:MM:SS"        TextSize 2  large clock
 *    y ≈ 50   "9th March 2026"  TextSize 1  human date
 *
 *  NAVIGATION:
 *    Long press  → SCREEN_MENU  (portrait rotation restored in ui.cpp)
 *    Short press → no-op
 *
 *  WIRING:
 *    DS3231 SDA → GPIO 21   (shared with SSD1306)
 *    DS3231 SCL → GPIO 22   (shared with SSD1306)
 *    VCC → 3.3V    GND → GND
 *
 *  DEPENDENCY:  adafruit/RTClib @ ^2.1.4  in platformio.ini
 * ╚══════════════════════════════════════════════════════════════════════════╝
 */

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <RTClib.h>

extern Adafruit_SSD1306 oled;
extern RTC_DS3231        rtc;   // defined in time_screen.cpp

// Landscape canvas dimensions (setRotation(0))
#define LS_CANVAS_W  128
#define LS_CANVAS_H   64

// ── API ─────────────────────────────────────────────────────────
// timeScreenInit() — call in setup() AFTER Wire.begin().
//   Returns true  : RTC found, time is valid.
//   Returns false : RTC not found on I2C bus (time screen will show 00:00:00).
bool timeScreenInit();

// renderTimeScreen() — draw one frame into the buffer.
//   Call after oled.clearDisplay(). Never calls oled.display().
//   Display must already be in setRotation(0) when called.
void renderTimeScreen();

// getTodayDate() — returns today as YYYYMMDD (e.g. 20260309).
//   Useful for streak tracking: compare stored date to today.
//   Returns 0 if RTC is not available.
uint32_t getTodayDate();

// ── Helpers (exposed for testing convenience) ────────────────────
const char *getDaySuffix(uint8_t day);
const char *getMonthName(uint8_t month);