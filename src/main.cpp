#include <Arduino.h>
#include "DHT.h"
#include "MQTT.h"
#include "LCD.h"

#define DHTPIN 13

MQTT mqtt = MQTT();
DHT dht = DHT(DHTPIN);
LCD lcd = LCD();

void setup() {
    Serial.begin(115200);
    delay(10);

    // Init
    mqtt.init();
    lcd.init();
}

void loop() {
    mqtt.loop();
    dht.sendData();
    delay(2000);
}
