#pragma once
/**
 * ╔══════════════════════════════════════════════════════════════════════════╗
 *  leaf_breeze.h  —  Vrindavan Breeze background layer
 *  Floating leaf silhouettes drawn BEFORE Krishna (background layer).
 *
 *  DEPENDENCIES: Adafruit_SSD1306 instance `oled` (extern, defined in main).
 *                CANVAS_W, CANVAS_H macros must be visible.
 *                TWO_PI macro must be visible.
 * ╚══════════════════════════════════════════════════════════════════════════╝
 */

#include <Arduino.h>
#include <Adafruit_SSD1306.h>

// ── Forward-declare the display so the header stays self-contained ──────────
extern Adafruit_SSD1306 oled;

// ── Canvas dimensions (must match main) ────────────────────────────────────
#ifndef CANVAS_W
  #define CANVAS_W  64
#endif
#ifndef CANVAS_H
  #define CANVAS_H 128
#endif

#ifndef TWO_PI
  #define TWO_PI 6.28318530718f
#endif

// ═══════════════════════════════════════════════════════════════
//  TUNING CONSTANTS
// ═══════════════════════════════════════════════════════════════
#define LEAF_COUNT         4      // 3–5  (keep ≤5 for RAM & FPS budget)

static const float LEAF_PERIOD     = 3.0f;   // seconds — seamless loop duration
static const float LEAF_SWAY_AMP   = 3.5f;   // px, peak horizontal oscillation
static const float LEAF_FALL_SPEED = 10.0f;  // px / second downward drift

// ═══════════════════════════════════════════════════════════════
//  LEAF SILHOUETTES  (3×3 bitmask, bit7 = leftmost pixel per row)
// ═══════════════════════════════════════════════════════════════
//   Shape A  ◇  simple diamond
//   Shape B  ◁  falling teardrop (angled right)
//   Shape C  ◸  broad left-leaning

static const uint8_t LEAF_SHAPE_A[3] = { 0x40, 0xE0, 0x40 };
static const uint8_t LEAF_SHAPE_B[3] = { 0x60, 0xC0, 0x40 };
static const uint8_t LEAF_SHAPE_C[3] = { 0xC0, 0xE0, 0x80 };

static const uint8_t * const LEAF_SHAPES[3] = {
  LEAF_SHAPE_A, LEAF_SHAPE_B, LEAF_SHAPE_C
};

// ═══════════════════════════════════════════════════════════════
//  LEAF STRUCT
// ═══════════════════════════════════════════════════════════════
struct Leaf {
  float   yPos;       // vertical position in canvas-px
  float   xBase;      // horizontal centre at sway=0
  float   phase;      // fall/wrap phase  [0, 2π)
  float   swayPhase;  // horizontal sway phase [0, 2π)
  uint8_t shapeIdx;   // silhouette index (0–2)
};

// ═══════════════════════════════════════════════════════════════
//  PUBLIC API
// ═══════════════════════════════════════════════════════════════

/** Call once at end of setup(). */
void initLeaves();

/**
 * Advance leaf physics.
 * @param dt  frame delta-time in seconds (same dt your main loop uses).
 */
void updateLeaves(float dt);

/**
 * Stamp leaf silhouettes onto the display buffer.
 * Call AFTER oled.clearDisplay() and BEFORE renderKrishna_noClr().
 */
void renderLeaves();