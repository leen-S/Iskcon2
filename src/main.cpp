/**
 * ╔══════════════════════════════════════════════════════════════════════════╗
 *  Krishna Muralidhara  v9.0  —  PlatformIO / ESP32
 *
 *  SCREENS
 *  ────────
 *  SCREEN_BOOT   → boot sequence (landscape greeting → butterfly)
 *  SCREEN_HOME   → Krishna animation + leaf breeze  (50 FPS delta-time)
 *  SCREEN_MENU   → 3-item button menu
 *  SCREEN_CHANT  → Japa counter 1–108
 *  SCREEN_STREAK → Persistent session streak (Preferences/NVS)
 *
 *  ──────
 *
 *  WIRING
 *  ───────
 *  SDA     → GPIO 21
 *  SCL     → GPIO 22
 *  VCC     → 3.3 V
 *  GND     → GND
 *  BUTTON  → GPIO 0 → GND  (active-LOW, internal pull-up)
 * ╚══════════════════════════════════════════════════════════════════════════╝
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>

#include "boot.h"
#include "Ui.h"
#include "chant.h"
#include "streak.h"
#include "leaf_breeze.h"

// ═══════════════════════════════════════════════════════════════
//  HARDWARE
// ═══════════════════════════════════════════════════════════════
#define OLED_HW_W   128
#define OLED_HW_H    64
#define OLED_ADDR   0x3C
#define OLED_RESET   -1
#define I2C_SDA      21
#define I2C_SCL      22
#define I2C_FREQ     400000UL

#define CANVAS_W     64
#define CANVAS_H    128

Adafruit_SSD1306 oled(OLED_HW_W, OLED_HW_H, &Wire, OLED_RESET);

// ═══════════════════════════════════════════════════════════════
//  FRAME TIMING
// ═══════════════════════════════════════════════════════════════
#define TARGET_FPS  50
#define FRAME_US    (1000000UL / TARGET_FPS)

static uint32_t prevFrameUs = 0;

// ═══════════════════════════════════════════════════════════════
//  KRISHNA BITMAP  39 × 107 px  (UNCHANGED)
// ═══════════════════════════════════════════════════════════════
#define BMP_W    39
#define BMP_H   107

static const uint8_t PROGMEM krishna_bmp[BMP_H][5] = {
  {0x00,0x18,0x00,0x00,0x00},{0x00,0x0F,0xC0,0x00,0x00},
  {0x00,0x0F,0xF0,0x00,0x00},{0x00,0x0F,0xF8,0x00,0x00},
  {0x00,0x0F,0xF8,0x00,0x00},{0x00,0x07,0xF8,0xC0,0x00},
  {0x00,0x07,0xF9,0xFC,0x00},{0x00,0x03,0xFB,0xFE,0x00},
  {0x00,0x01,0xFF,0xFF,0x80},{0x00,0x00,0x0F,0xFF,0x80},
  {0x00,0x00,0x1F,0xFF,0xC0},{0x00,0x00,0x1F,0xFF,0xC0},
  {0x00,0x00,0x1F,0xFF,0xC0},{0x00,0x00,0x1F,0xFF,0xC0},
  {0x00,0x00,0x1F,0xFF,0xE0},{0x00,0x00,0x0F,0xFF,0xE0},
  {0x00,0x00,0x07,0xFF,0xC0},{0x00,0x00,0x07,0xFF,0xC0},
  {0x00,0x00,0x07,0xFF,0xC0},{0x00,0x00,0x03,0xFF,0xC0},
  {0x00,0x00,0x01,0xFE,0x00},{0x00,0x00,0xF7,0xFF,0x00},
  {0x00,0x61,0xFF,0xFF,0x00},{0x01,0xFF,0xFF,0xFF,0xD0},
  {0x03,0xFF,0xFF,0xFF,0xFC},{0x7F,0xFF,0xFF,0xFF,0xFE},
  {0xFC,0xFF,0xFF,0xFF,0xFE},{0xC1,0xFF,0xFF,0xFF,0xFE},
  {0x00,0xFF,0xFF,0xFF,0xFE},{0x00,0xFF,0xFF,0xFF,0xFC},
  {0x00,0x7F,0xFF,0xFF,0xFC},{0x00,0x7F,0xF7,0xFF,0xF8},
  {0x00,0x7F,0xEF,0xFF,0xF8},{0x00,0x7F,0xFF,0xFF,0xF0},
  {0x00,0x3F,0xFF,0xFF,0xE0},{0x00,0x3F,0xEF,0xFF,0xE0},
  {0x00,0x1F,0xCF,0xFF,0xE0},{0x00,0x1F,0x87,0xFF,0x80},
  {0x00,0x0F,0x07,0xFF,0x00},{0x00,0x04,0x0F,0xFE,0x80},
  {0x00,0x00,0x0F,0xF9,0x80},{0x00,0x00,0x0F,0xF7,0x80},
  {0x00,0x00,0x0F,0xCF,0x00},{0x00,0x00,0x0F,0x3E,0x00},
  {0x00,0x00,0x1C,0xFF,0x00},{0x00,0x00,0x19,0xFF,0x00},
  {0x00,0x00,0x13,0xFF,0x80},{0x00,0x00,0x07,0xFF,0x80},
  {0x00,0x00,0x0F,0xFF,0x80},{0x00,0x00,0x1F,0xFF,0x80},
  {0x00,0x00,0x3F,0xFF,0x80},{0x00,0x00,0x3F,0xFF,0xC0},
  {0x00,0x00,0x5F,0xFF,0xC0},{0x00,0x01,0xBF,0xFF,0xC0},
  {0x00,0x01,0x7F,0xFF,0xE0},{0x00,0x02,0x7F,0xFF,0xE0},
  {0x00,0x06,0xFF,0xFF,0xE0},{0x00,0x0E,0xFF,0xFF,0xF0},
  {0x00,0x1E,0xFF,0xFF,0xF0},{0x00,0x1D,0xFF,0xFF,0xF0},
  {0x00,0x3D,0xFF,0xFF,0xF0},{0x00,0x7D,0xFF,0xFF,0xF8},
  {0x00,0xFD,0xFF,0xFF,0xF8},{0x00,0xF9,0xFF,0xFF,0xF8},
  {0x01,0xF9,0xFF,0xFF,0xF8},{0x03,0xF9,0xFF,0x7F,0xF8},
  {0x03,0xF9,0xFF,0x7F,0xFC},{0x07,0xF9,0xFF,0x7F,0xFC},
  {0x0F,0xF9,0xFF,0x3F,0xFC},{0x0F,0xF9,0xFF,0xBF,0xFC},
  {0x1F,0xF9,0xFF,0xDF,0xFC},{0x07,0xF1,0xFF,0x5F,0xFC},
  {0x03,0xF0,0xFF,0xCF,0xFC},{0x00,0xF0,0xFF,0xEF,0xFC},
  {0x00,0x70,0xFF,0xF7,0xFC},{0x00,0xF0,0xFF,0xF3,0xFC},
  {0x00,0x78,0xFF,0xF9,0xFC},{0x00,0x10,0x7E,0xFD,0xFC},
  {0x00,0x00,0x7E,0xFE,0xFC},{0x00,0x00,0x3E,0xFF,0x3C},
  {0x00,0x00,0x3E,0xFF,0x88},{0x00,0x00,0x3E,0xFF,0xC0},
  {0x00,0x00,0x1E,0xFF,0xE0},{0x00,0x00,0x1F,0xFF,0xF8},
  {0x00,0x00,0x0F,0xFF,0xF8},{0x00,0x00,0x0F,0xFF,0xF8},
  {0x00,0x00,0x03,0xFF,0xF0},{0x00,0x00,0x01,0xFF,0xE0},
  {0x00,0x00,0x00,0xFF,0xC0},{0x00,0x00,0x00,0xFF,0xC0},
  {0x00,0x00,0x00,0x7F,0x80},{0x00,0x00,0x00,0xFF,0x80},
  {0x00,0x00,0x00,0xFF,0x00},{0x00,0x00,0x00,0x7C,0x00},
  {0x00,0x00,0x00,0x7C,0x00},{0x00,0x00,0x00,0x7C,0x00},
  {0x00,0x00,0x00,0xFC,0x00},{0x00,0x00,0x00,0xFC,0x00},
  {0x00,0x00,0x00,0xFC,0x00},{0x00,0x00,0x01,0xFC,0x00},
  {0x00,0x00,0x01,0xFC,0x00},{0x00,0x00,0x03,0xFC,0x00},
  {0x00,0x00,0x07,0xFE,0x00},{0x00,0x00,0x0F,0xFE,0x00},
  {0x00,0x00,0x07,0x3F,0x00},{0x00,0x00,0x00,0x7F,0x80},
  {0x00,0x00,0x00,0x3F,0x00},
};

// ═══════════════════════════════════════════════════════════════
//  ANIMATION CONSTANTS
// ═══════════════════════════════════════════════════════════════
static const float BODY_AMP       = 2.2f;
static const float BODY_PERIOD    = 3.2f;
static const float FEATHER_EXTRA  = 0.6f;
static const float FEATHER_PERIOD = 2.1f;
static const int   FEATHER_ROWS   = 8;
static const int   FLUTE_ROW      = 26;
static const float SHIMMER_PERIOD = 0.9f;
static const float SHIMMER_THRESH = 0.6f;

static const int BASE_X = (CANVAS_W - BMP_W) / 2;
static const int BASE_Y = (CANVAS_H - BMP_H) / 2;

#ifndef TWO_PI
  #define TWO_PI 6.28318530718f
#endif

// ═══════════════════════════════════════════════════════════════
//  ANIMATION STATE
// ═══════════════════════════════════════════════════════════════
static float bodyPhase    = 0.0f;
static float featherPhase = 0.94f;
static float shimmerPhase = 0.0f;

// ═══════════════════════════════════════════════════════════════
//  renderKrishna_noClr  (UNCHANGED)
// ═══════════════════════════════════════════════════════════════
static inline void drawRow(int originX, int rowY, int row, int xShift) {
    if ((unsigned)rowY >= (unsigned)CANVAS_H) return;
    const int x0 = originX + xShift;
    for (int col = 0; col < BMP_W; col++) {
        if (pgm_read_byte(&krishna_bmp[row][col >> 3]) & (0x80 >> (col & 7))) {
            const unsigned px = (unsigned)(x0 + col);
            if (px < (unsigned)CANVAS_W)
                oled.drawPixel((int)px, rowY, SSD1306_WHITE);
        }
    }
}

static void renderKrishna_noClr(float bodySin, float featherSin, bool shimmerOn) {
    const float inv = 1.0f / (float)(BMP_H - 1);
    for (int row = 0; row < BMP_H; row++) {
        const float lev  = 1.0f - (float)row * inv;
        float sway = bodySin * BODY_AMP * lev;
        if (row < FEATHER_ROWS) {
            const float fb = 1.0f - (float)row * (1.0f / (float)FEATHER_ROWS);
            sway += featherSin * FEATHER_EXTRA * fb;
        }
        drawRow(BASE_X, BASE_Y + row, row, (int)lroundf(sway));
    }
    if (shimmerOn) {
        const float lev  = 1.0f - (float)FLUTE_ROW * inv;
        const int   sOff = (int)lroundf(bodySin * BODY_AMP * lev);
        const int   spkX = BASE_X + sOff - 2;
        const int   spkY = BASE_Y + FLUTE_ROW;
        if ((unsigned)spkY < (unsigned)CANVAS_H) {
            if ((unsigned)spkX       < (unsigned)CANVAS_W) oled.drawPixel(spkX,     spkY,   SSD1306_WHITE);
            if ((unsigned)(spkX + 1) < (unsigned)CANVAS_W) oled.drawPixel(spkX + 1, spkY,   SSD1306_WHITE);
            if ((unsigned)spkY + 1   < (unsigned)CANVAS_H) oled.drawPixel(spkX,   spkY + 1, SSD1306_WHITE);
        }
    }
}

// ═══════════════════════════════════════════════════════════════
//  SETUP  —  strict initialisation order
// ═══════════════════════════════════════════════════════════════
void setup() {
    // ── 1. I²C bus ──────────────────────────────────────────────
    Wire.begin(I2C_SDA, I2C_SCL);
    Wire.setClock(I2C_FREQ);

    // ── 2. OLED hardware init ────────────────────────────────────
    if (!oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
        // Halt silently — no Serial in production build
        for (;;) { }
    }

    // ── 3. Portrait rotation ─────────────────────────────────────
    oled.setRotation(1);

    // ── 4. Flush OLED RAM — eliminates garbage pixels on boot ────
    //       Two calls: first clears CPU buffer, second pushes over I²C
    oled.clearDisplay();
    oled.display();

    // ── 5. Brief settle — allows OLED power rail to stabilise ────
    //       Uses millis() spin NOT delay() to stay non-blocking
    {
        const uint32_t t0 = millis();
        while (millis() - t0 < 80) { /* spin */ }
    }

    // ── 6. Persistent data ───────────────────────────────────────
    streakInit();

    // ── 7. Subsystem init ────────────────────────────────────────
    initLeaves();
    uiInit();       // sets s_screen = SCREEN_BOOT
    bootInit();     // snapshots millis() for boot timer — must come LAST

    // ── 8. Timing gate baseline ──────────────────────────────────
    //       Set AFTER all init so first dt is never huge
    prevFrameUs = micros();
}

// ═══════════════════════════════════════════════════════════════
//  LOOP
// ═══════════════════════════════════════════════════════════════
void loop() {
    // ── 1. Poll button — BEFORE frame gate, catches every press ──
    const ScreenState screen = uiUpdate();

    // ── 2. Frame gate ────────────────────────────────────────────
    const uint32_t now     = micros();
    const uint32_t elapsed = now - prevFrameUs;
    if (elapsed < FRAME_US) return;
    prevFrameUs = now;

    // ── 3. Delta time (capped to absorb I²C stalls) ──────────────
    float dt = (float)elapsed * 1e-6f;
    const float DT_MAX = (float)FRAME_US * 3e-6f;
    if (dt > DT_MAX) dt = DT_MAX;

    // ── 4. Advance Krishna phases — always, even off HOME ────────
    //       Phases tick continuously so HOME resumes mid-sway cleanly
    bodyPhase    += dt * (TWO_PI / BODY_PERIOD);
    featherPhase += dt * (TWO_PI / FEATHER_PERIOD);
    shimmerPhase += dt * (TWO_PI / SHIMMER_PERIOD);
    if (bodyPhase    >= TWO_PI) bodyPhase    -= TWO_PI;
    if (featherPhase >= TWO_PI) featherPhase -= TWO_PI;
    if (shimmerPhase >= TWO_PI) shimmerPhase -= TWO_PI;

    const float bodySin    = sinf(bodyPhase);
    const float featherSin = sinf(featherPhase);
    const bool  shimmerOn  = (sinf(shimmerPhase) > SHIMMER_THRESH);

    // ── 5. Advance leaf physics — always ─────────────────────────
    updateLeaves(dt);

    // ── 6. ONE clear per frame ───────────────────────────────────
    oled.clearDisplay();

    // ── 7. Screen dispatch ───────────────────────────────────────
    switch (screen) {

        case SCREEN_BOOT: {
            const bool done = bootUpdate(dt);
            if (done) {
                // Transition: set HOME, render HOME this same frame
                // so there is zero blank frame between boot and home
                uiSetScreen(SCREEN_HOME);
                renderLeaves();
                renderKrishna_noClr(bodySin, featherSin, shimmerOn);
            } else {
                renderBoot();
            }
            break;
        }

        case SCREEN_HOME:
            renderLeaves();
            renderKrishna_noClr(bodySin, featherSin, shimmerOn);
            break;

        case SCREEN_MENU:
            renderMenu();
            break;

        case SCREEN_CHANT: {
            // Track previous chant state to fire streakRecord() exactly once
            static ChantState prevChantState = CHANT_IDLE;
            chantUpdate(buttonShortPress, buttonLongPress);
            if (chantState == CHANT_COMPLETE && prevChantState != CHANT_COMPLETE) {
                streakRecord();   // fires once on transition to COMPLETE
            }
            prevChantState = chantState;
            renderChant();
            break;
        }

        case SCREEN_STREAK:
            renderStreak();
            break;
    }

    // ── 8. ONE display push per frame ────────────────────────────
    oled.display();
}