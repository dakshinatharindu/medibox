#include "LCD.h"

LCD::LCD() : lcd(0x27, 20, 4) {}

void LCD::init() {
    this->lcd.init();
    this->lcd.backlight();
    this->lcd.setCursor(0, 0);
    this->lcd.print("Hello World!");
}
