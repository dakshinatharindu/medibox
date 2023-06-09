#include <Arduino.h>
#include <LiquidCrystal_I2C.h>


// #include "_MQTT.h"
// #include "_WiFi.h"
#include "MQTT.h"
#include "DHT.h"

#define DHTPIN 13

LiquidCrystal_I2C LCD = LiquidCrystal_I2C(0x27, 20, 4);
MQTT mqtt = MQTT();
DHT dht = DHT(DHTPIN);

void setup() {
    Serial.begin(115200);
    delay(10);
    LCD.init();
    LCD.backlight();
    LCD.setCursor(0, 0);
    LCD.print("Hello World!");
    mqtt.init();
 
    // setupWiFi();
    // setupMQTT();
    
}

void loop() {
    // if (!mqttClient.connected()) {
    //     connectBroker();
    // }
    // mqttClient.loop();

    mqtt.connect();
    dht.sendData();
    delay(2000);
}
