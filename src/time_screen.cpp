/**
 * time_screen.cpp  —  DS3231 RTC Time Applet  v10.0
 *
 *  renderTimeScreen() is called after oled.clearDisplay() in main loop.
 *  It does NOT call oled.display() — that happens once in main loop.
 *  Rotation is managed by ui.cpp on enter/exit transitions.
 *  This file only draws — it never changes rotation.
 */

#include "time_screen.h"

// ── RTC instance — defined here, declared extern in time_screen.h ──
RTC_DS3231 rtc;

// ── timeScreenInit ───────────────────────────────────────────────
bool timeScreenInit() {
    if (!rtc.begin()) {
        return false;   // RTC not found on I2C bus
    }
    // If RTC lost power, set to compile time as fallback.
    // Remove this line if you want the RTC to hold its time on reboot.
    if (rtc.lostPower()) {
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
    return true;
}

// ── getDaySuffix ─────────────────────────────────────────────────
// Returns "st", "nd", "rd", or "th" for any day 1–31.
// Handles the special teens (11th, 12th, 13th).
const char *getDaySuffix(uint8_t day) {
    if (day >= 11 && day <= 13) return "th";   // teen exception
    switch (day % 10) {
        case 1:  return "st";
        case 2:  return "nd";
        case 3:  return "rd";
        default: return "th";
    }
}

// ── getMonthName ─────────────────────────────────────────────────
const char *getMonthName(uint8_t month) {
    static const char * const MONTHS[13] = {
        "",           // index 0 unused — RTClib months are 1-based
        "January", "February", "March",    "April",
        "May",     "June",     "July",     "August",
        "September","October", "November", "December"
    };
    if (month < 1 || month > 12) return "???";
    return MONTHS[month];
}

// ── drawLandscapeCentred ─────────────────────────────────────────
// Centre a string horizontally on the 128px-wide landscape canvas
// at pixel row y. Uses getTextBounds() for pixel-accurate width.
static void drawLandscapeCentred(const char *str, uint8_t textSize, int16_t y) {
    oled.setTextSize(textSize);
    oled.setTextWrap(false);
    oled.setTextColor(SSD1306_WHITE);

    int16_t  bx, by;
    uint16_t bw, bh;
    oled.getTextBounds(str, 0, y, &bx, &by, &bw, &bh);
    if (bw == 0) bw = (uint16_t)(strlen(str) * 6u * textSize);

    const int16_t x = max((int16_t)0,
                          (int16_t)((LS_CANVAS_W - (int16_t)bw) / 2));
    oled.setCursor(x, y);
    oled.print(str);
}

// ── renderTimeScreen ─────────────────────────────────────────────
// Display is already in setRotation(0) when this runs —
// ui.cpp sets it on SCREEN_TIME entry and restores on exit.
//
// LANDSCAPE canvas: 128 px wide × 64 px tall
//
// Layout (TextSize 1 = 6×8 px char, TextSize 2 = 12×16 px):
//   y =  6   "Time"           TS1  8 px tall
//   y = 22   "HH:MM:SS"       TS2  16 px tall
//   y = 50   "Nth Month YYYY" TS1  8 px tall
void renderTimeScreen() {
    DateTime now = rtc.now();

    // ── Clock string "HH:MM:SS" ──────────────────────────────────
    char clockBuf[9];   // "HH:MM:SS\0"
    snprintf(clockBuf, sizeof(clockBuf), "%02u:%02u:%02u",
             now.hour(), now.minute(), now.second());

    // ── Date string "Nth Month YYYY" ─────────────────────────────
    char dateBuf[28];   // e.g. "21st September 2026\0" = 20 chars + margin
    snprintf(dateBuf, sizeof(dateBuf), "%u%s %s %u",
             (unsigned)now.day(),
             getDaySuffix(now.day()),
             getMonthName(now.month()),
             (unsigned)now.year());

    // ── Draw ─────────────────────────────────────────────────────
    // Label
    drawLandscapeCentred("Time", 1, 6);

    // Thin separator under label
    oled.drawFastHLine(20, 17, LS_CANVAS_W - 40, SSD1306_WHITE);

    // Large clock — TextSize 2 = 12px per char wide, 16px tall
    drawLandscapeCentred(clockBuf, 2, 22);

    // Thin separator above date
    oled.drawFastHLine(20, 43, LS_CANVAS_W - 40, SSD1306_WHITE);

    // Human date
    drawLandscapeCentred(dateBuf, 1, 50);

    // Exit hint bottom-right (small, unobtrusive)
    oled.setTextSize(1);
    oled.setTextColor(SSD1306_WHITE);
    oled.setCursor(LS_CANVAS_W - 48, LS_CANVAS_H - 8);
    oled.print(F("hold:menu"));
}