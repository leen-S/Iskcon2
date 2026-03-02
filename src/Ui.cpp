/**
 * ui.cpp  —  Screen State Machine + Button Handler  v5.0
 */

#include "Ui.h"
#include "chant.h"

// ── Public button event flags (cleared at top of uiUpdate()) ────
bool buttonShortPress = false;
bool buttonLongPress  = false;

// ── Private state ───────────────────────────────────────────────
static ScreenState s_screen  = SCREEN_HOME;
static uint8_t     s_menuSel = 0;

enum BtnPhase : uint8_t {
    BTN_IDLE, BTN_PRESS_PENDING, BTN_PRESSED, BTN_RELEASE_PENDING
};

static BtnPhase  s_btnPhase   = BTN_IDLE;
static uint32_t  s_btnTimerMs = 0;
static bool      s_longFired  = false;

enum BtnEvent : uint8_t { EVT_NONE, EVT_SHORT, EVT_LONG };

// ── Button polling ───────────────────────────────────────────────
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
            if (!rawLow)                                    s_btnPhase = BTN_IDLE;
            else if (now - s_btnTimerMs >= BTN_DEBOUNCE_MS) s_btnPhase = BTN_PRESSED;
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
            if (rawLow)                                       s_btnPhase = BTN_PRESSED;
            else if (now - s_btnTimerMs >= BTN_DEBOUNCE_MS) {
                s_btnPhase = BTN_IDLE;
                if (!s_longFired) return EVT_SHORT;
            }
            break;
    }
    return EVT_NONE;
}

// ── State machine transitions ────────────────────────────────────
static void handleEvent(BtnEvent evt) {
    if (evt == EVT_NONE) return;

    // Expose flags for sub-screens (chantUpdate etc.)
    if (evt == EVT_SHORT) buttonShortPress = true;
    if (evt == EVT_LONG)  buttonLongPress  = true;

    switch (s_screen) {

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
                if (target == SCREEN_CHANT)  chantEnter();
                // future: if (target == SCREEN_STREAK) streakEnter();
                s_screen = target;
                if (s_screen == SCREEN_HOME) s_menuSel = 0;
            }
            break;

        case SCREEN_CHANT:
            // Short press forwarded via buttonShortPress flag to chantUpdate()
            if (evt == EVT_LONG) {
                s_screen  = SCREEN_MENU;
                s_menuSel = 1;   // re-highlight "Chant"
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
    }
}

// ── Public API ───────────────────────────────────────────────────
void uiInit() {
    pinMode(BTN_PIN, INPUT_PULLUP);
    s_screen    = SCREEN_HOME;
    s_menuSel   = 0;
    s_btnPhase  = BTN_IDLE;
    s_longFired = false;
}

ScreenState uiUpdate() {
    buttonShortPress = false;   // clear flags at start of each frame
    buttonLongPress  = false;
    handleEvent(pollButton());
    return s_screen;
}

ScreenState uiCurrentScreen() {
    return s_screen;
}

// ── Menu layout constants ────────────────────────────────────────
static const uint8_t MENU_ROW_H     = 16;
static const uint8_t MENU_PADDING_X = 10;
static const uint8_t MENU_RECT_W    = CANVAS_W - (MENU_PADDING_X * 2);
static const uint8_t MENU_RECT_H    = 13;
static const uint8_t MENU_TEXT_OFFY =  3;
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
            oled.fillRoundRect(MENU_PADDING_X, rectY, MENU_RECT_W, MENU_RECT_H,
                               2, SSD1306_WHITE);
            oled.setTextColor(SSD1306_BLACK);
        } else {
            oled.setTextColor(SSD1306_WHITE);
        }
        oled.setCursor(MENU_PADDING_X + 4, rectY + MENU_TEXT_OFFY);
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
    oled.print(F("lng:home shrt:menu"));
}