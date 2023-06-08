#pragma once

#include <Arduino.h>
#include "DHTesp.h"

class DHT {
   private:
    DHTesp dhtSensor;

   public:
    DHT(const int pin);
    void sendData();
};
