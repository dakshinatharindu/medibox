#include <Arduino.h>

#include "DHT.h"
#include "LCD.h"
#include "LDR.h"
#include "MQTT.h"
#include "ServoMotor.h"

#define DHT_PIN 13
#define LDR_PIN 34
#define SERVO_PIN 18

MQTT mqtt = MQTT();
DHT dht = DHT(DHT_PIN, &mqtt);
LCD lcd = LCD();
LDR ldr = LDR(LDR_PIN, &mqtt);
ServoMotor servo = ServoMotor(SERVO_PIN);

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
    dht.loop();
    ldr.loop();
    servo.loop(&ldr.intensity, &mqtt.minAngle, &mqtt.contrlingFactor);
}
