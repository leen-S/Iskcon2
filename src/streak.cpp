/**
 * streak.cpp  —  Persistent Streak Tracking  v6.0
 */

#include "streak.h"

StreakData streakData = { 0, 0, 0 };

static Preferences prefs;

// ── streakInit ───────────────────────────────────────────────────
void streakInit() {
    prefs.begin(PREF_NAMESPACE, true);   // read-only open
    streakData.currentStreak  = prefs.getUInt(PREF_KEY_STREAK, 0);
    streakData.bestStreak     = prefs.getUInt(PREF_KEY_BEST,   0);
    streakData.totalSessions  = prefs.getUInt(PREF_KEY_TOTAL,  0);
    prefs.end();
}

// ── streakEnter ──────────────────────────────────────────────────
void streakEnter() {
    // Reload fresh data each time user views streak screen
    streakInit();
}

// ── streakRecord ─────────────────────────────────────────────────
// Call once when chant reaches 108.
void streakRecord() {
    streakData.currentStreak++;
    streakData.totalSessions++;
    if (streakData.currentStreak > streakData.bestStreak) {
        streakData.bestStreak = streakData.currentStreak;
    }
    streakSave();
}

// ── streakSave ───────────────────────────────────────────────────
void streakSave() {
    prefs.begin(PREF_NAMESPACE, false);   // read-write open
    prefs.putUInt(PREF_KEY_STREAK, streakData.currentStreak);
    prefs.putUInt(PREF_KEY_BEST,   streakData.bestStreak);
    prefs.putUInt(PREF_KEY_TOTAL,  streakData.totalSessions);
    prefs.end();
}

// ── renderStreak ─────────────────────────────────────────────────
void renderStreak() {
    oled.setTextSize(1);
    oled.setTextWrap(false);
    oled.setTextColor(SSD1306_WHITE);

    // Title
    const int16_t titleX = (CANVAS_W - (6 * 6)) / 2;   // "STREAK" = 6 chars
    oled.setCursor(titleX, 6);
    oled.print(F("STREAK"));
    oled.drawFastHLine(0, 17, CANVAS_W, SSD1306_WHITE);

    // Current streak — large
    char buf[12];
    snprintf(buf, sizeof(buf), "%lu", (unsigned long)streakData.currentStreak);
    oled.setTextSize(2);
    const int16_t numW = (int16_t)(strlen(buf) * 12);
    oled.setCursor((CANVAS_W - numW) / 2, 28);
    oled.print(buf);

    oled.setTextSize(1);
    oled.setTextColor(SSD1306_WHITE);

    // "sessions" label below number
    oled.setCursor((CANVAS_W - (8 * 6)) / 2, 50);
    oled.print(F("sessions"));

    // Divider
    oled.drawFastHLine(0, 62, CANVAS_W, SSD1306_WHITE);

    // Best
    oled.setCursor(4, 68);
    oled.print(F("Best:"));
    snprintf(buf, sizeof(buf), "%lu", (unsigned long)streakData.bestStreak);
    oled.setCursor(40, 68);
    oled.print(buf);

    // Total
    oled.setCursor(4, 82);
    oled.print(F("Total:"));
    snprintf(buf, sizeof(buf), "%lu", (unsigned long)streakData.totalSessions);
    oled.setCursor(40, 82);
    oled.print(buf);

    // Divider
    oled.drawFastHLine(0, 96, CANVAS_W, SSD1306_WHITE);

    // Exit hint
    oled.setCursor(2, CANVAS_H - 10);
    oled.print(F("tap:menu hold:home"));
}