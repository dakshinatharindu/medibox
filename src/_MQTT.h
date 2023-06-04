#include <PubSubClient.h>
#include <WiFi.h>

// Instantiate WiFiClient and PubSubClient
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Constants
const char *mqqttBroker = "test.mosquitto.org";
const int mqttPort = 1883;
const char *mqttClientID = "ESP32-Medibox";
const char *mqttSubTopic = "190622R/main_buzzer";

// Variables
bool main_switch = true;

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    char payloadCharArr[length];

    for (int i = 0; i < length; i++) {
        payloadCharArr[i] = (char)payload[i];
    }

    if (payloadCharArr[0] == 't') {
        main_switch = true;
    } else if (payloadCharArr[0] == 'f') {
        main_switch = false;
    }
    Serial.print("Main Switch: "); 
    Serial.println(main_switch);
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
