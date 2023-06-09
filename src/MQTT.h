#pragma once

#include <PubSubClient.h>
#include <WiFi.h>

class MQTT {
   private:
    // Attributes
    PubSubClient mqttClient;

    // Methods
    static void mqttCallback(char *topic, byte *payload, unsigned int length);

   public:
    // Attributes
    bool main_switch = false;

    // Methods
    MQTT(PubSubClient *mqttClient);
    void init();
    void loop();
};
