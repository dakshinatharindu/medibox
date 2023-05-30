#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

#include "_MQTT.h"
#include "_WiFi.h"

LiquidCrystal_I2C LCD = LiquidCrystal_I2C(0x27, 20, 4);

void setup() {
    Serial.begin(115200);
    delay(10);
    LCD.init();
    LCD.backlight();
    LCD.setCursor(0, 0);
    LCD.print("Hello World!");
    setupWiFi();
    setupMQTT();
}

void loop() {
    if (!mqttClient.connected()) {
        connectBroker();
    }
    mqttClient.loop();
}
