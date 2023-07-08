#include <Arduino.h>
#include <DHTesp.h>
#include <LiquidCrystal_I2C.h>
#include <PubSubClient.h>
#include <RTClib.h>
#include <Servo.h>
#include <WiFi.h>

// Pin definitions
#define DHT_PIN 13
#define LDR_PIN 34
#define SERVO_PIN 18
#define BUZZER_PIN 15

// Channels
#define BUZZER_CHANNEL 2
#define SERVO_CHANNEL 0

// Periods
#define DHT_PERIOD 5000
#define LDR_PERIOD 5000

// Alarm
struct Alarm {
    int hour;
    int minute;
    int second;
    bool isOn;
};

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
RTC_DS1307 rtc;

// Alarms
Alarm alarm1;
Alarm alarm2;
Alarm alarm3;


//////////////////// MQTT ////////////////////
void mqttCallback(char *topic, byte *payload, unsigned int length) {
    char payloadCharArr[length];

    for (int i = 0; i < length; i++) {
        payloadCharArr[i] = (char)payload[i];
    }
    String payloadStr = String(payloadCharArr).substring(0, length);
    Serial.println(payloadStr);
    if (strcmp(topic, mainBuzzerTopic) == 0) {
        mainSwitch = payloadStr.toInt();
        if (mainSwitch)
            ledcWriteTone(BUZZER_CHANNEL, 1000);
        else
            ledcWrite(BUZZER_CHANNEL, 0);
    } else if (strcmp(topic, schedulerTopic) == 0) {
        // TODO: Implement scheduler
    } else if (strcmp(topic, minAngleTopic) == 0) {
        minAngle = payloadStr.toInt();
    } else if (strcmp(topic, contrlingFactorTopic) == 0) {
        contrlingFactor = payloadStr.toFloat();
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

void lcdShowTime() {
    DateTime now = rtc.now();

    // print year, month, day
    lcd.setCursor(0, 0);
    lcd.print(' ');
    lcd.print(' ');
    lcd.print(' ');
    lcd.print(' ');
    lcd.print(' ');
    lcd.print(now.year(), DEC);
    lcd.print('-');
    if (now.month() < 10) lcd.print('0');
    lcd.print(now.month(), DEC);
    lcd.print('-');
    if (now.day() < 10) lcd.print('0');
    lcd.print(now.day(), DEC);
    lcd.print(' ');
    lcd.print(' ');
    lcd.print(' ');
    lcd.print(' ');
    lcd.print(' ');

    // print hour, minute, second
    lcd.setCursor(0, 1);
    lcd.print(' ');
    lcd.print(' ');
    lcd.print(' ');
    lcd.print(' ');
    lcd.print(' ');
    lcd.print(' ');
    if (now.hour() < 10) lcd.print('0');
    lcd.print(now.hour(), DEC);
    lcd.print(':');
    if (now.minute() < 10) lcd.print('0');
    lcd.print(now.minute(), DEC);
    lcd.print(':');
    if (now.second() < 10) lcd.print('0');
    lcd.print(now.second(), DEC);
    lcd.print(' ');
    lcd.print(' ');
    lcd.print(' ');
    lcd.print(' ');
    lcd.print(' ');
    lcd.print(' ');
}

//////////////////// SERVO ////////////////////
void servoInit() { servo.attach(SERVO_PIN, SERVO_CHANNEL); }

void servoLoop() {
    int angle =
        (int)(minAngle + (180 - minAngle) * intensity * contrlingFactor);
    servo.write(angle);
    // TODO: smooth the servo movement
}

//////////////////// BUZZER ////////////////////
void buzzerInit() {
    ledcSetup(BUZZER_CHANNEL, 1000, 16);
    ledcAttachPin(BUZZER_PIN, 2);
}

void buzzerLoop() {
    if (mainSwitch)
        ledcWriteTone(BUZZER_CHANNEL, 1000);
    else
        ledcWrite(BUZZER_CHANNEL, 0);
}

//////////////////// RTC ////////////////////
void rtcInit() {
    if (!rtc.begin()) {
        Serial.println("Couldn't find RTC");
        Serial.flush();
        abort();
    }
}

//////////////////// SETUP ////////////////////
void setup() {
    // Init Serial
    Serial.begin(115200);
    delay(10);

    // Init Objects
    rtcInit();
    lcdInit();
    dhtInit();
    ldrInit();
    mqttInit();
    servoInit();
    buzzerInit();
}

//////////////////// LOOP ////////////////////
void loop() {
    mqttLoop();
    dhtLoop();
    ldrLoop();
    servoLoop();
    lcdShowTime();
}
