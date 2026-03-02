#pragma once
/**
 * ╔══════════════════════════════════════════════════════════════════════════╗
 *  streak.h  —  Persistent Daily Streak Tracking  v6.0
 *
 *  Uses ESP32 Preferences (NVS) for persistence across reboots.
 *
 *  LOGIC
 *  ──────
 *  A "streak day" is recorded when the user completes at least one full
 *  chant round (108 beads). The streak increments if the last completion
 *  was yesterday; resets to 1 if it was today (already counted); resets
 *  to 0 if more than 1 day has passed.
 *
 *  Days are tracked as Unix-day index: millis()-based day counter since
 *  boot. For real calendar tracking, integrate with an RTC module.
 *  Without RTC, streaks persist across reboots but days are estimated
 *  by session count (one session = one streak unit).
 * ╚══════════════════════════════════════════════════════════════════════════╝
 */

#include <Arduino.h>
#include <Preferences.h>
#include <Adafruit_SSD1306.h>

extern Adafruit_SSD1306 oled;

#ifndef CANVAS_W
  #define CANVAS_W  64
#endif
#ifndef CANVAS_H
  #define CANVAS_H 128
#endif

// ── Preferences namespace ────────────────────────────────────────
#define PREF_NAMESPACE   "krishna"
#define PREF_KEY_STREAK  "streak"
#define PREF_KEY_BEST    "best"
#define PREF_KEY_TOTAL   "total"

// ── Streak data ──────────────────────────────────────────────────
struct StreakData {
    uint32_t currentStreak;   // consecutive sessions
    uint32_t bestStreak;      // all-time best
    uint32_t totalSessions;   // lifetime total chant completions
};

extern StreakData streakData;

// ── API ─────────────────────────────────────────────────────────
void streakInit();      // load from Preferences — call in setup()
void streakEnter();     // call when navigating TO streak screen
void streakRecord();    // call when chant completes (108 reached)
void streakSave();      // persist to NVS
void renderStreak();    // draw streak screen — no oled.display()