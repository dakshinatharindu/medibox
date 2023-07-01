#include <Arduino.h>
#include "MQTT.h"

#define LDR_PERIOD 5000

class LDR {
   private:
    uint8_t pin;
    MQTT* mqtt;
    unsigned long time;
    
   public:
    LDR(uint8_t pin, MQTT* mqtt);
    void loop();
    float intensity;
};