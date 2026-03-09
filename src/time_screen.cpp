/**
 * time_screen.cpp  —  DS3231 RTC Time Applet  v11.0
 *
 *  v11.0 FIX LOG
 *  ──────────────
 *  PROBLEM (v10): rtc.adjust() was always called — resetting clock on every
 *                 upload or reboot because lostPower() was not checked.
 *
 *  FIX: rtc.adjust() now fires ONLY when rtc.lostPower() returns true.
 *       lostPower() reads the DS3231's Oscillator Stop Flag (OSF bit).
 *       The DS3231 sets this bit when its coin cell is removed or dead.
 *       On a normal ESP32 reboot, the DS3231 keeps running on its battery —
 *       OSF is clear → lostPower() = false → adjust() is NOT called.
 *
 *  I2C SHARING
 *  ────────────
 *  Wire.begin(21, 22) in main setup() initialises the shared bus once.
 *  Both oled.begin() (SSD1306 at 0x3C) and rtc.begin() (DS3231 at 0x68)
 *  attach to the same Wire instance. Each I2C transaction is atomic —
 *  no conflict occurs because setup() and loop() are single-threaded.
 *
 *  SERIAL DEBUG
 *  ─────────────
 *  Serial.begin() is called in timeScreenInit() only if not already started.
 *  All Serial output uses F() macro to keep strings in flash, not RAM.
 *
 *  ARCHITECTURE CONTRACT
 *  ──────────────────────
 *  renderTimeScreen() : called after clearDisplay(), never calls display().
 *  timeScreenInit()   : called in setup() after Wire.begin().
 *  getTodayDate()     : pure computation — safe to call every frame if needed.
 *  No delay() anywhere in this file.
 */

#include "time_screen.h"

// ── RTC singleton — extern declared in time_screen.h ────────────
RTC_DS3231 rtc;

// ── Module state ────────────────────────────────────────────────
static bool s_rtcOk = false;   // true once rtc.begin() succeeds

// ════════════════════════════════════════════════════════════════
//  timeScreenInit
//  Call in setup() AFTER Wire.begin(21, 22).
//  Wire must be initialised before this — the SSD1306 and DS3231
//  share the same bus, so Wire.begin() owns that responsibility.
// ════════════════════════════════════════════════════════════════
bool timeScreenInit() {
    // Start Serial for debug output if not already running.
    // 115200 matches monitor_speed in platformio.ini.
    if (!Serial) {
        Serial.begin(115200);
    }

    // ── 1. Detect RTC on I2C bus ─────────────────────────────────
    if (!rtc.begin()) {
        Serial.println(F("[RTC] ERROR: DS3231 not found on I2C bus."));
        Serial.println(F("[RTC] Check wiring: SDA=GPIO21, SCL=GPIO22, VCC=3.3V"));
        s_rtcOk = false;
        return false;
    }

    // ── 2. Only set time if the RTC lost power ───────────────────
    //       lostPower() reads the DS3231 Oscillator Stop Flag (OSF).
    //       OSF is set by hardware when VCC and VBAT both drop below
    //       threshold. On a normal ESP32 reboot, the coin cell keeps
    //       the DS3231 running — OSF stays clear — we skip adjust().
    if (rtc.lostPower()) {
        Serial.println(F("[RTC] Power loss detected (OSF bit set)."));
        Serial.println(F("[RTC] Setting time to firmware compile timestamp."));
        Serial.println(F("[RTC] Replace battery if this happens repeatedly."));
        // __DATE__ / __TIME__ are baked in at compile time.
        // This is a fallback — not a wall-clock-accurate source.
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    } else {
        Serial.println(F("[RTC] Power was maintained. Time is valid."));
    }

    // ── 3. Log current time to Serial ────────────────────────────
    DateTime now = rtc.now();
    char logBuf[40];
    snprintf(logBuf, sizeof(logBuf),
             "[RTC] Current time: %04u-%02u-%02u %02u:%02u:%02u",
             (unsigned)now.year(),  (unsigned)now.month(),  (unsigned)now.day(),
             (unsigned)now.hour(),  (unsigned)now.minute(), (unsigned)now.second());
    Serial.println(logBuf);
    Serial.println(F("[RTC] Initialized successfully."));

    s_rtcOk = true;
    return true;
}

// ════════════════════════════════════════════════════════════════
//  getTodayDate
//  Returns today as YYYYMMDD (e.g. 20260309) for streak tracking.
//  Comparing this value across days detects a day boundary without
//  needing a midnight interrupt or timer.
//  Returns 0 if RTC was not found.
// ════════════════════════════════════════════════════════════════
uint32_t getTodayDate() {
    if (!s_rtcOk) return 0;
    DateTime now = rtc.now();
    return (uint32_t)now.year()  * 10000UL
         + (uint32_t)now.month() *   100UL
         + (uint32_t)now.day();
}

// ════════════════════════════════════════════════════════════════
//  getDaySuffix
//  "st" / "nd" / "rd" / "th" with correct teen handling.
//  11th, 12th, 13th are always "th" regardless of last digit.
// ════════════════════════════════════════════════════════════════
const char *getDaySuffix(uint8_t day) {
    if (day >= 11 && day <= 13) return "th";
    switch (day % 10) {
        case 1:  return "st";
        case 2:  return "nd";
        case 3:  return "rd";
        default: return "th";
    }
}

// ════════════════════════════════════════════════════════════════
//  getMonthName
//  RTClib months are 1-based (1 = January).
// ════════════════════════════════════════════════════════════════
const char *getMonthName(uint8_t month) {
    static const char * const MONTHS[13] = {
        "",
        "January", "February", "March",     "April",
        "May",     "June",     "July",      "August",
        "September","October", "November",  "December"
    };
    if (month < 1 || month > 12) return "???";
    return MONTHS[month];
}

// ════════════════════════════════════════════════════════════════
//  drawLandscapeCentred  (internal)
//  Centres a string on the 128px-wide landscape canvas at row y.
//  Uses getTextBounds() for pixel-accurate width.
// ════════════════════════════════════════════════════════════════
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

// ════════════════════════════════════════════════════════════════
//  renderTimeScreen
//  Called after oled.clearDisplay() — never calls oled.display().
//  Display is already in setRotation(0) — managed by ui.cpp.
//
//  LANDSCAPE canvas: 128 px wide × 64 px tall
//
//  Layout:
//    y =  6   "Time"            TS1  (8 px tall)
//    y = 17   separator line
//    y = 22   "HH:MM:SS"        TS2  (16 px tall)
//    y = 43   separator line
//    y = 50   "9th March 2026"  TS1  (8 px tall)
//    y = 56   "hold:menu"       hint, bottom-right
//
//  If RTC is not available, shows "--:--:--" and "No RTC" in place
//  of real data — screen degrades gracefully, never crashes.
// ════════════════════════════════════════════════════════════════
void renderTimeScreen() {
    char clockBuf[9];   // "HH:MM:SS\0"
    char dateBuf[28];   // "21st September 2026\0"

    if (s_rtcOk) {
        DateTime now = rtc.now();
        snprintf(clockBuf, sizeof(clockBuf), "%02u:%02u:%02u",
                 now.hour(), now.minute(), now.second());
        snprintf(dateBuf, sizeof(dateBuf), "%u%s %s %u",
                 (unsigned)now.day(),
                 getDaySuffix(now.day()),
                 getMonthName(now.month()),
                 (unsigned)now.year());
    } else {
        // Graceful degradation — no crash, obvious indicator
        snprintf(clockBuf, sizeof(clockBuf), "--:--:--");
        snprintf(dateBuf,  sizeof(dateBuf),  "No RTC");
    }

    // ── Draw ─────────────────────────────────────────────────────
    drawLandscapeCentred("Time", 1, 6);
    oled.drawFastHLine(20, 17, LS_CANVAS_W - 40, SSD1306_WHITE);
    drawLandscapeCentred(clockBuf, 2, 22);
    oled.drawFastHLine(20, 43, LS_CANVAS_W - 40, SSD1306_WHITE);
    drawLandscapeCentred(dateBuf, 1, 50);

    // Exit hint — bottom-right, small, unobtrusive
    oled.setTextSize(1);
    oled.setTextColor(SSD1306_WHITE);
    oled.setCursor(LS_CANVAS_W - 48, LS_CANVAS_H - 8);
    oled.print(F("hold:menu"));
}