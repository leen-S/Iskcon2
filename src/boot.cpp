/**
 * boot.cpp  —  Boot Sequence implementation  v8.0
 *
 *  CHANGE LOG  v7 → v8
 *  ─────────────────────
 *  - BOOT_GREETING and BOOT_SIGNATURE render in LANDSCAPE (setRotation(0))
 *    In landscape: physical display is 128px wide × 64px tall
 *  - setRotation(1) restored at the moment BOOT_BUTTERFLY begins —
 *    exactly once, inside bootUpdate() on the state transition.
 *    This means butterfly + all subsequent screens are portrait.
 *  - drawCenteredText() now takes a canvasW parameter so it works
 *    correctly in both landscape (128) and portrait (64) modes.
 *  - No extra oled.display() calls added — still one per frame from main.
 *  - All other files unchanged.
 */

#include "boot.h"
#include <math.h>

// ── Butterfly bitmap ─────────────────────────────────────────────
#define BFLY_W  7
#define BFLY_H  5

static const uint8_t BFLY_OPEN[BFLY_H] = {
    0b10101000,
    0b11111000,
    0b01010000,
    0b01110000,
    0b00100000,
};

static const uint8_t BFLY_CLOSED[BFLY_H] = {
    0b01010000,
    0b01110000,
    0b01010000,
    0b01110000,
    0b00100000,
};

struct Butterfly {
    float xPos;
    float yBase;
    float swayPhase;
    float wingPhase;
};

// ── Module state ─────────────────────────────────────────────────
static BootState s_bootState  = BOOT_GREETING;
static uint32_t  s_stageStart = 0;
static Butterfly s_bfly;

// ─────────────────────────────────────────────────────────────────
//  ROTATION CONSTANTS
//
//  LANDSCAPE (setRotation(0)):   width = 128 px,  height = 64 px
//  PORTRAIT  (setRotation(1)):   width =  64 px,  height = 128 px
//
//  GREETING + SIGNATURE stages → landscape  → restore to portrait
//  before BUTTERFLY so every subsequent render is portrait.
// ─────────────────────────────────────────────────────────────────
#define LS_W  128   // landscape canvas width
#define LS_H   64   // landscape canvas height

// ── bootInit ─────────────────────────────────────────────────────
void bootInit() {
    s_bootState      = BOOT_GREETING;
    s_stageStart     = millis();
    s_bfly.xPos      = -(float)(BFLY_W + 4);
    s_bfly.yBase     = (float)(CANVAS_H / 2);   // CANVAS_H = portrait 128
    s_bfly.swayPhase = 0.0f;
    s_bfly.wingPhase = 0.0f;
}

// ── bootUpdate ───────────────────────────────────────────────────
bool bootUpdate(float dt) {
    const uint32_t now     = millis();
    const uint32_t elapsed = now - s_stageStart;

    switch (s_bootState) {

        case BOOT_GREETING:
            if (elapsed >= BOOT_GREETING_MS) {
                s_bootState  = BOOT_SIGNATURE;
                s_stageStart = now;
            }
            break;

        case BOOT_SIGNATURE:
            if (elapsed >= BOOT_SIGNATURE_MS) {
                // ── Restore portrait before butterfly ──────────────
                // Done here in the state machine — exactly once,
                // not every frame — so renderBoot() butterfly case
                // always finds the display already in portrait mode.
                oled.setRotation(1);
                oled.clearDisplay();   // flush landscape buffer cleanly

                s_bootState      = BOOT_BUTTERFLY;
                s_stageStart     = now;
                s_bfly.xPos      = -(float)(BFLY_W + 4);
                s_bfly.swayPhase = 0.0f;
                s_bfly.wingPhase = 0.0f;
            }
            break;

        case BOOT_BUTTERFLY: {
            const float totalDist = (float)(CANVAS_W + BFLY_W + 8);
            const float speed     = totalDist / ((float)BOOT_BUTTERFLY_MS * 1e-3f);

            s_bfly.xPos      += speed * dt;
            s_bfly.swayPhase += dt * (TWO_PI / BFLY_SWAY_PERIOD);
            s_bfly.wingPhase += dt * (TWO_PI / BFLY_WING_PERIOD);

            if (s_bfly.swayPhase >= TWO_PI) s_bfly.swayPhase -= TWO_PI;
            if (s_bfly.wingPhase >= TWO_PI) s_bfly.wingPhase -= TWO_PI;

            if (elapsed >= BOOT_BUTTERFLY_MS ||
                s_bfly.xPos > (float)(CANVAS_W + BFLY_W + 2)) {
                s_bootState = BOOT_DONE;
            }
            break;
        }

        case BOOT_DONE:
            return true;
    }

    return false;
}

// ── Render helpers ───────────────────────────────────────────────

/**
 * Draw a string centred horizontally within canvasW at pixel row y.
 * canvasW = 128 in landscape, 64 in portrait.
 * Uses getTextBounds() for pixel-accurate width.
 */
static void drawCenteredText(const char *str, uint8_t textSize,
                              int16_t y, int16_t canvasW) {
    oled.setTextSize(textSize);
    oled.setTextWrap(false);
    oled.setTextColor(SSD1306_WHITE);

    int16_t  bx, by;
    uint16_t bw, bh;
    oled.getTextBounds(str, 0, y, &bx, &by, &bw, &bh);
    if (bw == 0) bw = (uint16_t)(strlen(str) * 6u * textSize);

    const int16_t x = max((int16_t)0, (int16_t)((canvasW - (int16_t)bw) / 2));
    oled.setCursor(x, y);
    oled.print(str);
}

static void drawButterfly(int cx, int cy, bool wingOpen) {
    const uint8_t *bmp = wingOpen ? BFLY_OPEN : BFLY_CLOSED;
    for (int row = 0; row < BFLY_H; row++) {
        const int py = cy + row;
        if ((unsigned)py >= (unsigned)CANVAS_H) continue;
        const uint8_t bits = bmp[row];
        for (int col = 0; col < BFLY_W; col++) {
            if (bits & (0x80 >> col)) {
                const int px = cx + col;
                if ((unsigned)px < (unsigned)CANVAS_W)
                    oled.drawPixel(px, py, SSD1306_WHITE);
            }
        }
    }
}

// ── renderBoot ───────────────────────────────────────────────────
// Called AFTER oled.clearDisplay() in main loop — never calls display().
//
// GREETING / SIGNATURE:  sets rotation(0) at start, draws in landscape.
//                         Rotation is NOT restored here — it is restored
//                         once inside bootUpdate() on the SIGNATURE→BUTTERFLY
//                         transition, keeping this function side-effect-free.
//
// BUTTERFLY / DONE:       display is already portrait (setRotation(1)),
//                         draws using CANVAS_W / CANVAS_H as normal.
void renderBoot() {
    switch (s_bootState) {

        // ── Landscape text screens ────────────────────────────────
        case BOOT_GREETING: {
            // Switch to landscape for this frame's render
            oled.setRotation(0);

            // "Happy B'day Dhanu" — single centred line
            // Landscape: 128×64. textSize(1) = 8px tall.
            // Single line vertically centred: y = (64 - 8) / 2 = 28
            drawCenteredText("Happy B'day Dhanu", 1,
                             (LS_H - 8) / 2, LS_W);
            break;
        }

        case BOOT_SIGNATURE: {
            oled.setRotation(0);

            // "By Leen" centred with decorative lines
            const int16_t y = (LS_H - 8) / 2;   // 28 px
            oled.drawFastHLine(8, y - 5,  LS_W - 16, SSD1306_WHITE);
            drawCenteredText("By Leen", 1, y, LS_W);
            oled.drawFastHLine(8, y + 11, LS_W - 16, SSD1306_WHITE);
            break;
        }

        // ── Portrait butterfly ────────────────────────────────────
        // Display is already rotation(1) — restored in bootUpdate()
        case BOOT_BUTTERFLY: {
            const float swaySin = sinf(s_bfly.swayPhase);
            const float wingSin = sinf(s_bfly.wingPhase);

            const int cx = (int)lroundf(s_bfly.xPos);
            const int cy = (int)lroundf(s_bfly.yBase + swaySin * BFLY_SWAY_AMP)
                           - (BFLY_H / 2);

            drawButterfly(cx, cy, wingSin >= 0.0f);

            if (cx > 6) {
                const int trailY = (int)lroundf(
                    s_bfly.yBase + sinf(s_bfly.swayPhase - 0.5f) * BFLY_SWAY_AMP
                );
                if ((unsigned)trailY < (unsigned)CANVAS_H) {
                    const int t1 = cx - 3;
                    const int t2 = cx - 6;
                    if ((unsigned)t1 < (unsigned)CANVAS_W)
                        oled.drawPixel(t1, trailY, SSD1306_WHITE);
                    if ((unsigned)t2 < (unsigned)CANVAS_W)
                        oled.drawPixel(t2, trailY, SSD1306_WHITE);
                }
            }
            break;
        }

        case BOOT_DONE:
            break;
    }
}