#include "ServoMotor.h"

ServoMotor::ServoMotor(uint8_t pin) {
    servo.attach(pin);
}

void ServoMotor::loop(float *intensity, int *minAngle, float *contrlingFactor) {
    int angle = (int)(*minAngle + (180 - *minAngle) * *intensity * *contrlingFactor);
    servo.write(angle);
    // TODO: smooth the servo movement
}