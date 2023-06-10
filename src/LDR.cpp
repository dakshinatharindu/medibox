#include "LDR.h"

const char* LDRTopic = "190622R/light";
// const float gama = 0.7;
// const float rl10 = 50;

LDR::LDR(uint8_t pin, MQTT* mqtt) {
    this->pin = pin;
    this->mqtt = mqtt;
    this->time = millis();

    pinMode(this->pin, INPUT);
}

void LDR::sendData() {
    if (millis() - this->time > LDR_PERIOD) {
        int analogValue = analogRead(this->pin);
        // analogValue = map(analogValue, 4095, 0, 1024, 0);
        // float voltage = analogValue / 1024. * 5;
        // float resistance = 2000 * voltage / (1 - voltage / 5);
        // float lux = pow(rl10 * 1e3 * pow(10, gama) / resistance, (1 / gama));

        float lux = 1 - analogValue / 4095.;

        this->mqtt->publish(LDRTopic, lux);
        this->time = millis();
        Serial.println(lux);
    }
}