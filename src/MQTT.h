#pragma once

#include <PubSubClient.h>
#include <WiFi.h>

class MQTT {
   private:
    // Attributes
    WiFiClient espClient;
    PubSubClient mqttClient;

    // Methods
    static void mqttCallback(char *topic, byte *payload, unsigned int length);

   public:
    // Attributes
    static bool mainSwitch;
    static int minAngle;
    static float contrlingFactor;

    // Methods
    MQTT();
    void init();
    void loop();
    void publish(const char *topic, const char *payload);
    void publish(const char *topic, const int payload);
    void publish(const char *topic, const float payload);
};
