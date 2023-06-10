#include "MQTT.h"

const char *SSID = "Wokwi-GUEST";
const char *PASSWORD = "";
const char *mqqttBroker = "test.mosquitto.org";
const int mqttPort = 1883;
const char *mqttClientID = "ESP32-Medibox";
const char *mainBuzzerTopic = "190622R/main_buzzer";
const char *schedulerTopic = "190622R/scheduler";

MQTT::MQTT(): mqttClient(this->espClient) {
    this->mqttClient.setServer(mqqttBroker, mqttPort);
    this->mqttClient.setCallback(this->mqttCallback);
}

void MQTT::init() {
    WiFi.begin(SSID, PASSWORD);

    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(1000);
    }

    Serial.print("Connected to ");
    Serial.println(SSID);
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

void MQTT::mqttCallback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);

    Serial.print("Message:");
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();
    Serial.println("-----------------------");
}

void MQTT::loop() {
    while (!this->mqttClient.connected()) {
        Serial.print("Attempting MQTT connection...");
        if (this->mqttClient.connect(mqttClientID)) {
            Serial.println("connected");
            this->mqttClient.subscribe(mainBuzzerTopic);
            this->mqttClient.subscribe(schedulerTopic);
        } else {
            Serial.print("failed, rc=");
            Serial.print(this->mqttClient.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }

    this->mqttClient.loop();
}

void MQTT::publish(const char *topic, const char *payload) {
    this->mqttClient.publish(topic, payload);
}

void MQTT::publish(const char *topic, const int payload) {
    this->mqttClient.publish(topic, String(payload).c_str());
}

void MQTT::publish(const char *topic, const float payload) {
    this->mqttClient.publish(topic, String(payload).c_str());
}