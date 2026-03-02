/**
 * leaf_breeze.cpp  —  Vrindavan Breeze  v6.0
 */

#include "leaf_breeze.h"
#include <math.h>

static Leaf leaves[LEAF_COUNT];

void initLeaves() {
    const float phaseStep = TWO_PI / (float)LEAF_COUNT;
    const float xPos[5]   = { 6.0f, 15.0f, 46.0f, 54.0f, 25.0f };
    for (int i = 0; i < LEAF_COUNT; i++) {
        leaves[i].phase     = phaseStep * (float)i;
        leaves[i].swayPhase = phaseStep * (float)i + 0.8f;
        leaves[i].xBase     = xPos[i % 5];
        leaves[i].yPos      = ((float)CANVAS_H / (float)LEAF_COUNT) * (float)i;
        leaves[i].shapeIdx  = (uint8_t)(i % 3);
    }
}

void updateLeaves(float dt) {
    const float spd = TWO_PI / LEAF_PERIOD;
    for (int i = 0; i < LEAF_COUNT; i++) {
        Leaf &L = leaves[i];
        L.phase     += dt * spd;
        L.swayPhase += dt * spd;
        if (L.phase     >= TWO_PI) L.phase     -= TWO_PI;
        if (L.swayPhase >= TWO_PI) L.swayPhase -= TWO_PI;
        L.yPos += LEAF_FALL_SPEED * dt;
        if (L.yPos > (float)(CANVAS_H + 3))
            L.yPos = -3.0f - (float)(i * 2);
    }
}

static void drawLeafShape(int cx, int cy, uint8_t idx) {
    const uint8_t *s = LEAF_SHAPES[idx];
    for (int row = 0; row < 3; row++) {
        const int py = cy - 1 + row;
        if ((unsigned)py >= (unsigned)CANVAS_H) continue;
        const uint8_t bits = s[row];
        for (int col = 0; col < 3; col++) {
            if (bits & (0x80 >> col)) {
                const int px = cx - 1 + col;
                if ((unsigned)px < (unsigned)CANVAS_W)
                    oled.drawPixel(px, py, SSD1306_WHITE);
            }
        }
    }
}

void renderLeaves() {
    for (int i = 0; i < LEAF_COUNT; i++) {
        const Leaf &L = leaves[i];
        if (L.yPos < -3.0f || L.yPos >= (float)(CANVAS_H + 3)) continue;
        const float sway = sinf(L.swayPhase) * LEAF_SWAY_AMP;
        drawLeafShape((int)lroundf(L.xBase + sway), (int)lroundf(L.yPos), L.shapeIdx);
    }
}