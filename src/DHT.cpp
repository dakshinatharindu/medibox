#include "DHT.h"

DHT::DHT(const int pin) {
    this->dhtSensor.setup(pin, DHTesp::DHT22);
}

void DHT::sendData() {
    TempAndHumidity dhtData = this->dhtSensor.getTempAndHumidity();
    Serial.println("Temp: " + String(dhtData.temperature, 2) + "°C");
    Serial.println("Humidity: " + String(dhtData.humidity, 1) + "%");
    Serial.println("---");
}