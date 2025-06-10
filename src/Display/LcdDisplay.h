#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include <LiquidCrystal.h>

#define LCD_DISPLAY_BLINK_INTERVAL_MS 1000

class LcdDisplay {
    constexpr static uint8_t SCREEN_WIDTH = 16;
    constexpr static uint8_t SCREEN_HEIGHT = 2;

    uint32_t tickMs = 0;
    bool isBlinking = false;

public:
    LiquidCrystal *screen;

    explicit LcdDisplay(LiquidCrystal *lcd) : screen(lcd) {
        screen->setCursor(0, 0);
        screen->println("Initializing...");
    }

    void clear() const {
        screen->clear();
    }

    void setBlinking(const bool isBlinking) {
        this->isBlinking = isBlinking;
    }

    void printCentered(const String &text) const {
        setCursorToCenter(text.length());
        screen->print(text);
    }

    void print(const String &text) const {
        screen->print(text);
    }

    void setCursorToCenter(const uint8_t amountOfChars) const {
        setCursorFromTopLeft((SCREEN_WIDTH - amountOfChars) / 2);
    }

    void setCursorFromTopLeft(const uint8_t x, const uint8_t y = 0) const {
        screen->setCursor(x, y);
    }

    void setCursorToLine(const uint8_t charOffset = 0, const uint8_t line = 0) const {
        screen->setCursor(charOffset, line);
    }

    void setCursorToLineRight(const String &text, const uint8_t line = 0) const {
        screen->setCursor(SCREEN_WIDTH - text.length(), line);
    }
};

#endif //LCD_DISPLAY_H
