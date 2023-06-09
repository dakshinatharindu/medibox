#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <PubSubClient.h>
#include <WiFi.h>

// #include "_MQTT.h"
// #include "_WiFi.h"
#include "DHT.h"
#include "MQTT.h"

#define DHTPIN 13

WiFiClient espClient;
PubSubClient mqttClient(espClient);
LiquidCrystal_I2C LCD = LiquidCrystal_I2C(0x27, 20, 4);
MQTT mqtt = MQTT(&mqttClient);
DHT dht = DHT(DHTPIN);

void setup() {
    Serial.begin(115200);
    delay(10);
    LCD.init();
    LCD.backlight();
    LCD.setCursor(0, 0);
    LCD.print("Hello World!");
    mqtt.init();
}

void loop() {
    mqtt.loop();
    dht.sendData();
    delay(2000);
}
