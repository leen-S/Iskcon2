/**
 * ui.cpp  —  Screen State Machine + Button Handler  v10.0
 *
 *  v10.0: SCREEN_TIME added.
 *  Rotation is managed here at state transitions:
 *    → SCREEN_TIME entry:  oled.setRotation(0)  landscape
 *    ← SCREEN_TIME exit:   oled.setRotation(1)  portrait
 *  renderTimeScreen() itself never touches rotation.
 */

#include "Ui.h"
#include "chant.h"
#include "streak.h"

// ── Public frame-scoped event flags ─────────────────────────────
bool buttonShortPress = false;
bool buttonLongPress  = false;

// ── Private state ───────────────────────────────────────────────
static ScreenState s_screen  = SCREEN_BOOT;
static uint8_t     s_menuSel = 0;

// ── Button state machine ─────────────────────────────────────────
enum BtnPhase : uint8_t {
    BTN_IDLE,
    BTN_PRESS_PENDING,
    BTN_PRESSED,
    BTN_RELEASE_PENDING
};

static BtnPhase  s_btnPhase   = BTN_IDLE;
static uint32_t  s_btnTimerMs = 0;
static bool      s_longFired  = false;

enum BtnEvent : uint8_t { EVT_NONE, EVT_SHORT, EVT_LONG };

// ── pollButton ───────────────────────────────────────────────────
static BtnEvent pollButton() {
    const bool     rawLow = (digitalRead(BTN_PIN) == LOW);
    const uint32_t now    = millis();

    switch (s_btnPhase) {

        case BTN_IDLE:
            if (rawLow) {
                s_btnPhase   = BTN_PRESS_PENDING;
                s_btnTimerMs = now;
                s_longFired  = false;
            }
            break;

        case BTN_PRESS_PENDING:
            if (!rawLow) {
                s_btnPhase = BTN_IDLE;
            } else if (now - s_btnTimerMs >= BTN_DEBOUNCE_MS) {
                s_btnPhase = BTN_PRESSED;
            }
            break;

        case BTN_PRESSED:
            if (!s_longFired && (now - s_btnTimerMs >= BTN_LONGPRESS_MS)) {
                s_longFired = true;
                return EVT_LONG;
            }
            if (!rawLow) {
                s_btnPhase   = BTN_RELEASE_PENDING;
                s_btnTimerMs = now;
            }
            break;

        case BTN_RELEASE_PENDING:
            if (rawLow) {
                s_btnPhase = BTN_PRESSED;
            } else if (now - s_btnTimerMs >= BTN_DEBOUNCE_MS) {
                s_btnPhase = BTN_IDLE;
                if (!s_longFired) return EVT_SHORT;
            }
            break;
    }
    return EVT_NONE;
}

// ── Rotation helpers ─────────────────────────────────────────────
// Called exactly once per transition — not every frame.
static void enterLandscape() {
    oled.setRotation(0);   // 128 wide × 64 tall
    oled.clearDisplay();   // flush portrait buffer — prevents artifact bleed
}

static void exitLandscape() {
    oled.setRotation(1);   // 64 wide × 128 tall  (portrait default)
    oled.clearDisplay();   // flush landscape buffer
}

// ── handleEvent ──────────────────────────────────────────────────
static void handleEvent(BtnEvent evt) {
    if (evt == EVT_NONE) return;

    if (evt == EVT_SHORT) buttonShortPress = true;
    if (evt == EVT_LONG)  buttonLongPress  = true;

    switch (s_screen) {

        case SCREEN_BOOT:
            if (evt == EVT_SHORT || evt == EVT_LONG) {
                s_screen = SCREEN_HOME;
            }
            break;

        case SCREEN_HOME:
            if (evt == EVT_SHORT) {
                s_screen  = SCREEN_MENU;
                s_menuSel = 0;
            }
            break;

        case SCREEN_MENU:
            if (evt == EVT_SHORT) {
                s_menuSel = (s_menuSel + 1) % MENU_ITEM_COUNT;
            } else if (evt == EVT_LONG) {
                ScreenState target = MENU_TARGETS[s_menuSel];
                // Sub-screen entry setup
                if (target == SCREEN_CHANT)  chantEnter();
                if (target == SCREEN_STREAK) streakEnter();
                if (target == SCREEN_TIME)   enterLandscape();
                s_screen = target;
                if (s_screen == SCREEN_HOME) s_menuSel = 0;
            }
            break;

        case SCREEN_CHANT:
            // Short press handled via buttonShortPress in loop()
            if (evt == EVT_LONG) {
                s_screen  = SCREEN_MENU;
                s_menuSel = 1;
            }
            break;

        case SCREEN_STREAK:
            if (evt == EVT_SHORT) {
                s_screen  = SCREEN_MENU;
                s_menuSel = 2;
            } else if (evt == EVT_LONG) {
                s_screen  = SCREEN_HOME;
                s_menuSel = 0;
            }
            break;

        case SCREEN_TIME:
            // Short press: no-op
            if (evt == EVT_LONG) {
                exitLandscape();          // restore portrait before menu draws
                s_screen  = SCREEN_MENU;
                s_menuSel = 3;            // re-highlight "Time" in menu
            }
            break;
    }
}

// ── Public API ───────────────────────────────────────────────────
void uiInit() {
    pinMode(BTN_PIN, INPUT_PULLUP);
    s_screen    = SCREEN_BOOT;
    s_menuSel   = 0;
    s_btnPhase  = BTN_IDLE;
    s_longFired = false;
}

ScreenState uiUpdate() {
    buttonShortPress = false;
    buttonLongPress  = false;
    handleEvent(pollButton());
    return s_screen;
}

ScreenState uiCurrentScreen() {
    return s_screen;
}

void uiSetScreen(ScreenState s) {
    s_screen = s;
}

// ── Menu layout ──────────────────────────────────────────────────
// Portrait canvas: 64 wide × 128 tall.
// 4 items × 16px rows = 64px total menu height.
static const uint8_t MENU_ROW_H     = 14;   // slightly tighter for 4 items
static const uint8_t MENU_PADDING_X =  8;
static const uint8_t MENU_RECT_W    = CANVAS_W - (MENU_PADDING_X * 2);
static const uint8_t MENU_RECT_H    = 11;
static const uint8_t MENU_TEXT_OFFY =  2;
static const uint8_t MENU_TOTAL_H   = MENU_ITEM_COUNT * MENU_ROW_H;
static const uint8_t MENU_ORIGIN_Y  = (CANVAS_H - MENU_TOTAL_H) / 2;
static const uint8_t TITLE_Y        = 4;

void renderMenu() {
    oled.setTextSize(1);
    oled.setTextWrap(false);
    oled.setTextColor(SSD1306_WHITE);
    oled.setCursor(MENU_PADDING_X, TITLE_Y);
    oled.print(F("MENU"));
    oled.drawFastHLine(0, TITLE_Y + 10, CANVAS_W, SSD1306_WHITE);

    for (uint8_t i = 0; i < MENU_ITEM_COUNT; i++) {
        const int16_t rectY = MENU_ORIGIN_Y + (int16_t)(i * MENU_ROW_H);
        if (i == s_menuSel) {
            oled.fillRoundRect(MENU_PADDING_X, rectY, MENU_RECT_W,
                               MENU_RECT_H, 2, SSD1306_WHITE);
            oled.setTextColor(SSD1306_BLACK);
        } else {
            oled.setTextColor(SSD1306_WHITE);
        }
        oled.setCursor(MENU_PADDING_X + 3, rectY + MENU_TEXT_OFFY);
        oled.print(MENU_LABELS[i]);
    }

    oled.setTextColor(SSD1306_WHITE);
    oled.setCursor(2, CANVAS_H - 10);
    oled.print(F("shrt:next lng:sel"));
}

void renderPlaceholder(const char *title) {
    oled.setTextSize(1);
    oled.setTextWrap(false);
    oled.setTextColor(SSD1306_WHITE);
    const int16_t ty = (CANVAS_H / 2) - 12;
    oled.setCursor(MENU_PADDING_X, ty);
    oled.print(title);
    oled.drawFastHLine(0, ty + 10, CANVAS_W, SSD1306_WHITE);
    oled.setCursor(MENU_PADDING_X, ty + 14);
    oled.print(F("Coming soon"));
    oled.setCursor(2, CANVAS_H - 10);
    oled.print(F("hold:home shrt:menu"));
}