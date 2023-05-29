#include <Arduino.h>

#include "_MQTT.h"
#include "_WiFi.h"

void setup() {
    Serial.begin(115200);
    setupWiFi();
    setupMQTT();
}

void loop() {
    if (!mqttClient.connected()) {
        connectBroker();
    }
    mqttClient.loop();
}
