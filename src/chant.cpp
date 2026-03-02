/**
 * chant.cpp  —  Chant Screen implementation
 */

#include "chant.h"

ChantState chantState = CHANT_IDLE;
uint16_t   chantCount = 0;

void chantEnter() {
    chantState = CHANT_IDLE;
    chantCount = 0;
}

void chantUpdate(bool shortPress, bool longPress) {
    (void)longPress;  // screen transition handled by ui.cpp

    switch (chantState) {
        case CHANT_IDLE:
            if (shortPress) {
                chantState = CHANT_COUNTING;
                chantCount = 1;
                if (chantCount >= CHANT_TARGET) chantState = CHANT_COMPLETE;
            }
            break;

        case CHANT_COUNTING:
            if (shortPress) {
                chantCount++;
                if (chantCount >= CHANT_TARGET) {
                    chantCount = CHANT_TARGET;
                    chantState = CHANT_COMPLETE;
                }
            }
            break;

        case CHANT_COMPLETE:
            break;  // locked
    }
}

// ── Render helpers ───────────────────────────────────────────────

static void drawCentred(const char *str, uint8_t textSize, int8_t offsetY = 0) {
    oled.setTextSize(textSize);
    oled.setTextWrap(false);
    oled.setTextColor(SSD1306_WHITE);
    const int16_t charW = 6 * textSize;
    const int16_t charH = 8 * textSize;
    const int16_t strW  = (int16_t)(strlen(str) * charW);
    const int16_t x     = (CANVAS_W - strW) / 2;
    const int16_t y     = (CANVAS_H - charH) / 2 + offsetY;
    oled.setCursor(x, y);
    oled.print(str);
}

static void drawProgressBar() {
    const uint8_t margin  = 8;
    const uint8_t barY    = 100;
    const uint8_t barH    = 4;
    const uint8_t barMaxW = CANVAS_W - (margin * 2);
    oled.drawRect(margin, barY, barMaxW, barH, SSD1306_WHITE);
    const uint8_t filled = (uint8_t)((uint32_t)chantCount * barMaxW / CHANT_TARGET);
    if (filled > 0) oled.fillRect(margin, barY, filled, barH, SSD1306_WHITE);
}

void renderChant() {
    switch (chantState) {

        case CHANT_IDLE:
            drawCentred("Let's Go", 1, -8);
            oled.setTextSize(1);
            oled.setTextColor(SSD1306_WHITE);
            oled.setCursor(8, (CANVAS_H / 2) + 6);
            oled.print(F("press to begin"));
            {
                const uint8_t margin  = 8;
                const uint8_t barMaxW = CANVAS_W - (margin * 2);
                oled.drawRect(margin, 100, barMaxW, 4, SSD1306_WHITE);
            }
            break;

        case CHANT_COUNTING: {
            char buf[4];
            snprintf(buf, sizeof(buf), "%u", chantCount);
            drawCentred(buf, 2, -12);
            oled.setTextSize(1);
            oled.setTextColor(SSD1306_WHITE);
            const int16_t subX = (CANVAS_W - (5 * 6)) / 2;
            oled.setCursor(subX, (CANVAS_H / 2) + 8);
            oled.print(F("/ 108"));
            drawProgressBar();
            break;
        }

        case CHANT_COMPLETE:
            oled.setTextSize(1);
            oled.setTextColor(SSD1306_WHITE);
            oled.setCursor((CANVAS_W - 18) / 2, 18);
            oled.print(F("108"));
            drawCentred("Hari Bol", 1, 0);
            {
                const uint8_t margin  = 8;
                const uint8_t barMaxW = CANVAS_W - (margin * 2);
                oled.fillRect(margin, 100, barMaxW, 4, SSD1306_WHITE);
            }
            oled.setTextSize(1);
            oled.setCursor(4, CANVAS_H - 10);
            oled.print(F("hold: back"));
            break;
    }
}