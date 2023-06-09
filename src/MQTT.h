#pragma once

#include <PubSubClient.h>
#include <WiFi.h>

class MQTT {
   private:
    WiFiClient espClient;
    PubSubClient mqttClient;
    bool main_switch = false;

    static void mqttCallback(char *topic, byte *payload, unsigned int length);

   public:
    MQTT();
    void init();
    void connect();
};
