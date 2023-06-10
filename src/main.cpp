#include <Arduino.h>
#include "DHT.h"
#include "MQTT.h"
#include "LCD.h"
#include "LDR.h"

#define DHT_PIN 13
#define LDR_PIN 34

MQTT mqtt = MQTT();
DHT dht = DHT(DHT_PIN, &mqtt);
LCD lcd = LCD();
LDR ldr = LDR(LDR_PIN, &mqtt);

void setup() {
    // Init Serial
    Serial.begin(115200);
    delay(10);

    // Init modules
    mqtt.init();
    lcd.init();
}

void loop() {
    mqtt.loop();
    dht.sendData();
    ldr.sendData();
}
