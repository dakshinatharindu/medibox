#pragma once

#include <Arduino.h>
#include <DHTesp.h>
#include "MQTT.h"

#define DHT_PERIOD 5000

class DHT {
   private:
    DHTesp dhtSensor;
    MQTT* mqtt;
    unsigned long time;

   public:
    DHT(const int pin, MQTT* mqtt);
    void loop();
};
