#include <Arduino.h>
#include <DHTesp.h>
#include <LiquidCrystal_I2C.h>
#include <PubSubClient.h>
#include <Servo.h>
#include <WiFi.h>

// Pin definitions
#define DHT_PIN 13
#define LDR_PIN 34
#define SERVO_PIN 18

// Periods
#define DHT_PERIOD 5000
#define LDR_PERIOD 5000

// Global Constants
const char *SSID = "Wokwi-GUEST";
const char *PASSWORD = "";
const char *mqqttBroker = "test.mosquitto.org";
const int mqttPort = 1883;
const char *mqttClientID = "ESP32-Medibox";
const char *mainBuzzerTopic = "190622R/main_buzzer";
const char *schedulerTopic = "190622R/scheduler";
const char *minAngleTopic = "190622R/min_angle";
const char *contrlingFactorTopic = "190622R/contrling_factor";
const char *tempTopic = "190622R/temp";
const char *humidityTopic = "190622R/humidity";
const char *LDRTopic = "190622R/light";
// const float gama = 0.7;
// const float rl10 = 50;

// Global variables
unsigned long dhtTime;
unsigned long ldrTime;
bool mainSwitch = false;
int minAngle = 30;
float contrlingFactor = 0.75;
float intensity;

// Objects
DHTesp dhtSensor;
WiFiClient espClient;
PubSubClient mqttClient(espClient);
LiquidCrystal_I2C lcd(0x27, 20, 4);
Servo servo;

// ServoMotor servo = ServoMotor(SERVO_PIN);

//////////////////// MQTT ////////////////////
void mqttCallback(char *topic, byte *payload, unsigned int length) {
    char payloadCharArr[length];

    for (int i = 0; i < length; i++) {
        payloadCharArr[i] = (char)payload[i];
    }

    if (strcmp(topic, mainBuzzerTopic) == 0) {
        if (payloadCharArr[0] == 't') {
            mainSwitch = true;
        } else if (payloadCharArr[0] == 'f') {
            mainSwitch = false;
        }
    } else if (strcmp(topic, schedulerTopic) == 0) {
        // TODO: Implement scheduler
    } else if (strcmp(topic, minAngleTopic) == 0) {
        minAngle = atoi(payloadCharArr);
    } else if (strcmp(topic, contrlingFactorTopic) == 0) {
        contrlingFactor = atof(payloadCharArr);
    }
}

void mqttInit() {
    mqttClient.setServer(mqqttBroker, mqttPort);
    mqttClient.setCallback(mqttCallback);
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

void mqttLoop() {
    while (!mqttClient.connected()) {
        Serial.print("Attempting MQTT connection...");
        if (mqttClient.connect(mqttClientID)) {
            Serial.println("connected");
            mqttClient.subscribe(mainBuzzerTopic);
            mqttClient.subscribe(schedulerTopic);
            mqttClient.subscribe(minAngleTopic);
            mqttClient.subscribe(contrlingFactorTopic);
        } else {
            Serial.print("failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
    mqttClient.loop();
}

//////////////////// DHT ////////////////////
void dhtInit() {
    dhtTime = millis();
    dhtSensor.setup(DHT_PIN, DHTesp::DHT22);
}

void dhtLoop() {
    // print data periodically
    if (millis() - dhtTime > DHT_PERIOD) {
        TempAndHumidity dhtData = dhtSensor.getTempAndHumidity();
        // publish data to MQTT
        mqttClient.publish(tempTopic, String(dhtData.temperature).c_str());
        mqttClient.publish(humidityTopic, String(dhtData.humidity).c_str());
        dhtTime = millis();
    }
}

//////////////////// LDR ////////////////////
void ldrInit() {
    pinMode(LDR_PIN, INPUT);
    ldrTime = millis();
}

void ldrLoop() {
    int analogValue = analogRead(LDR_PIN);
    // analogValue = map(analogValue, 4095, 0, 1024, 0);
    // float voltage = analogValue / 1024. * 5;
    // float resistance = 2000 * voltage / (1 - voltage / 5);
    // float lux = pow(rl10 * 1e3 * pow(10, gama) / resistance, (1 / gama));

    intensity = 1 - analogValue / 4095.;

    if (millis() - ldrTime > LDR_PERIOD) {
        mqttClient.publish(LDRTopic, String(intensity).c_str());
        ldrTime = millis();
    }
}

//////////////////// LCD ////////////////////
void lcdInit() {
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Hello World!");
}

//////////////////// SERVO ////////////////////
void servoInit() { servo.attach(SERVO_PIN); }

void servoLoop() {
    int angle =
        (int)(minAngle + (180 - minAngle) * intensity * contrlingFactor);
    servo.write(angle);
    // TODO: smooth the servo movement
}

//////////////////// SETUP ////////////////////
void setup() {
    // Init Serial
    Serial.begin(115200);
    delay(10);

    // Init Objects
    dhtInit();
    ldrInit();
    mqttInit();
    lcdInit();
    servoInit();
}

//////////////////// LOOP ////////////////////
void loop() {
    mqttLoop();
    dhtLoop();
    ldrLoop();
    servoLoop();
}
