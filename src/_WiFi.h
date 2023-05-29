#include <WiFi.h>

// Constants
const char *SSID = "Wokwi-GUEST";
const char *PASSWORD = "";

void setupWiFi() {
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