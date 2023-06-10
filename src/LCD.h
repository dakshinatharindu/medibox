#pragma once

#include <LiquidCrystal_I2C.h>

class LCD {
   private:
    LiquidCrystal_I2C lcd;
   public:
    LCD();
    void init();
};
