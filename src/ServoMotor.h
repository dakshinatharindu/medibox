#include <Arduino.h>

#include "Servo.h"

class ServoMotor {
   private:
    Servo servo;

   public:
    ServoMotor(uint8_t pin);
    void loop(float *intensity, int *minAngle, float *contrlingFactor);
};