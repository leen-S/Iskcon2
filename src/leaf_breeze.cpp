/**
 * ╔══════════════════════════════════════════════════════════════════════════╗
 *  leaf_breeze.cpp  —  Vrindavan Breeze implementation
 * ╚══════════════════════════════════════════════════════════════════════════╝
 */

#include "leaf_breeze.h"
#include <math.h>

// ── Module-private state ────────────────────────────────────────────────────
static Leaf leaves[LEAF_COUNT];

// ═══════════════════════════════════════════════════════════════
//  initLeaves
// ═══════════════════════════════════════════════════════════════
void initLeaves() {
  // Evenly distribute phase so leaves never bunch up in the 3-second loop.
  const float phaseStep = TWO_PI / (float)LEAF_COUNT;

  // X base positions: hug canvas edges, keep centre clear for Krishna.
  // Indices wrap if LEAF_COUNT > 5.
  const float xPos[5] = { 6.0f, 15.0f, 46.0f, 54.0f, 25.0f };

  for (int i = 0; i < LEAF_COUNT; i++) {
    leaves[i].phase     = phaseStep * (float)i;
    // Sway phase offset (+0.8 rad) makes horizontal motion feel independent
    // from fall without needing a second timer.
    leaves[i].swayPhase = phaseStep * (float)i + 0.8f;
    leaves[i].xBase     = xPos[i % 5];
    // Spread y evenly so they don't all enter the screen simultaneously on boot.
    leaves[i].yPos      = ((float)CANVAS_H / (float)LEAF_COUNT) * (float)i;
    leaves[i].shapeIdx  = (uint8_t)(i % 3);
  }
}

// ═══════════════════════════════════════════════════════════════
//  updateLeaves
// ═══════════════════════════════════════════════════════════════
void updateLeaves(float dt) {
  const float phaseSpeed = TWO_PI / LEAF_PERIOD;  // rad/s — computed once

  for (int i = 0; i < LEAF_COUNT; i++) {
    Leaf &L = leaves[i];

    // Advance both phases at the same rate → loop-locked, no drift
    L.phase     += dt * phaseSpeed;
    L.swayPhase += dt * phaseSpeed;

    // Wrap to [0, 2π) — same pattern as Krishna body/feather phases
    if (L.phase     >= TWO_PI) L.phase     -= TWO_PI;
    if (L.swayPhase >= TWO_PI) L.swayPhase -= TWO_PI;

    // Fall downward
    L.yPos += LEAF_FALL_SPEED * dt;

    // Wrap: re-enter from top with a per-leaf stagger to avoid pop-in sync
    if (L.yPos > (float)(CANVAS_H + 3)) {
      L.yPos = -3.0f - (float)(i * 2);
    }
  }
}

// ═══════════════════════════════════════════════════════════════
//  drawLeafShape  (private helper)
// ═══════════════════════════════════════════════════════════════
static void drawLeafShape(int cx, int cy, uint8_t shapeIdx) {
  const uint8_t *shape = LEAF_SHAPES[shapeIdx];
  // 3×3 stamp centred on (cx, cy)
  for (int row = 0; row < 3; row++) {
    const int py = cy - 1 + row;
    if ((unsigned)py >= (unsigned)CANVAS_H) continue;
    const uint8_t bits = shape[row];
    for (int col = 0; col < 3; col++) {
      if (bits & (0x80 >> col)) {
        const int px = cx - 1 + col;
        if ((unsigned)px < (unsigned)CANVAS_W)
          oled.drawPixel(px, py, SSD1306_WHITE);
      }
    }
  }
}

// ═══════════════════════════════════════════════════════════════
//  renderLeaves
// ═══════════════════════════════════════════════════════════════
void renderLeaves() {
  for (int i = 0; i < LEAF_COUNT; i++) {
    const Leaf &L = leaves[i];

    // Skip leaves entirely off-screen (avoids pixel loop overhead)
    if (L.yPos < -3.0f || L.yPos >= (float)(CANVAS_H + 3)) continue;

    // ONE sinf() per leaf — LEAF_COUNT total, never per-pixel
    const float sway = sinf(L.swayPhase) * LEAF_SWAY_AMP;

    // Single lroundf() at the end — no truncation asymmetry, no dither
    const int cx = (int)lroundf(L.xBase + sway);
    const int cy = (int)lroundf(L.yPos);

    drawLeafShape(cx, cy, L.shapeIdx);
  }
}