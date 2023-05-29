#include <PubSubClient.h>
#include <WiFi.h>

// Instantiate WiFiClient and PubSubClient
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Constants
const char *mqqttBroker = "test.mosquitto.org";
const int mqttPort = 1883;
const char *mqttClientID = "ESP32-Medibox";
const char *mqttSubTopic = "dakshina-pub";

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    char payloadCharArr[length];

    for (int i = 0; i < length; i++) {
        payloadCharArr[i] = (char)payload[i];
    }

    // Handle payload
}

void setupMQTT() {
    mqttClient.setServer(mqqttBroker, mqttPort);
    mqttClient.setCallback(mqttCallback);
}

void connectBroker() {
    while (!mqttClient.connected()) {
        Serial.print("Attempting to connect to MQTT Broker....");
        if (mqttClient.connect(mqttClientID)) {
            Serial.println("Connected");
            mqttClient.subscribe(mqttSubTopic);
        } else {
            Serial.println("Failed");
            delay(5000);
        }
    }
}
