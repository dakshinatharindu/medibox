#include "DHT.h"

const char* tempTopic = "190622R/temp";
const char* humidityTopic = "190622R/humidity";

DHT::DHT(const int pin, MQTT* mqtt) {
    this->mqtt = mqtt;
    this->dhtSensor.setup(pin, DHTesp::DHT22);
    this->time = millis();
}

void DHT::sendData() {
    // print data periodically
    if (millis() - this->time > DHT_PERIOD) {
        TempAndHumidity dhtData = this->dhtSensor.getTempAndHumidity();
        // publish data to MQTT
        this->mqtt->publish(tempTopic, dhtData.temperature);
        this->mqtt->publish(humidityTopic, dhtData.humidity);
        this->time = millis();
    }
}