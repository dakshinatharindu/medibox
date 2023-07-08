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
    int isOn;
    int hour;
    int minute;
};

// Global Constants
const char *SSID = "Wokwi-GUEST";
const char *PASSWORD = "";
const char *mqqttBroker = "test.mosquitto.org";
const int mqttPort = 1883;
const char *mqttClientID = "ESP32-Medibox";
const char *mainBuzzerTopic = "190622R/main_buzzer";
const char *alarmsTopic = "190622R/alarms";
const char *minAngleTopic = "190622R/min_angle";
const char *contrlingFactorTopic = "190622R/contrling_factor";
const char *tempTopic = "190622R/temp";
const char *humidityTopic = "190622R/humidity";
const char *LDRTopic = "190622R/light";

// Global variables
unsigned long dhtTime;
unsigned long ldrTime;
bool mainSwitch = false;
bool schedule = false;
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
Alarm alarms[3];

// Function declarations
void lcdShowAlarm();

//////////////////// ALARM ////////////////////
int cmpfunc(const void *a, const void *b) {
    Alarm *alarm1 = (Alarm *)a;
    Alarm *alarm2 = (Alarm *)b;
    int compare1 = alarm1->isOn * 2000 + alarm1->hour * 60 + alarm1->minute;
    int compare2 = alarm2->isOn * 2000 + alarm2->hour * 60 + alarm2->minute;
    return compare1 - compare2;
}

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
    } else if (strcmp(topic, alarmsTopic) == 0) {
        schedule = payloadStr.substring(0, 1).toInt();

        alarms[0].hour = payloadStr.substring(2, 4).toInt();
        alarms[0].minute = payloadStr.substring(4, 6).toInt();
        alarms[0].isOn = payloadStr.substring(1, 2).toInt();

        alarms[1].hour = payloadStr.substring(7, 9).toInt();
        alarms[1].minute = payloadStr.substring(9, 11).toInt();
        alarms[1].isOn = payloadStr.substring(6, 7).toInt();

        alarms[2].hour = payloadStr.substring(12, 14).toInt();
        alarms[2].minute = payloadStr.substring(14, 16).toInt();
        alarms[2].isOn = payloadStr.substring(11, 12).toInt();

        lcdShowAlarm();

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
            mqttClient.subscribe(alarmsTopic);
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
    static uint8_t second = 0;

    if (second != now.second()) {
        // print year, month, day
        lcd.setCursor(0, 0);
        lcd.print("     ");
        lcd.print(now.year(), DEC);
        lcd.print('-');
        if (now.month() < 10) lcd.print('0');
        lcd.print(now.month(), DEC);
        lcd.print('-');
        if (now.day() < 10) lcd.print('0');
        lcd.print(now.day(), DEC);
        lcd.print("     ");

        // print hour, minute, second
        lcd.setCursor(0, 1);
        lcd.print("      ");
        if (now.hour() < 10) lcd.print('0');
        lcd.print(now.hour(), DEC);
        lcd.print(':');
        if (now.minute() < 10) lcd.print('0');
        lcd.print(now.minute(), DEC);
        lcd.print(':');
        if (now.second() < 10) lcd.print('0');
        lcd.print(now.second(), DEC);
        lcd.print("      ");

        second = now.second();
    }
}

void lcdShowAlarm() {
    lcd.setCursor(0, 2);
    lcd.print("  UPCOMING SCHEDULE ");
    lcd.setCursor(0, 3);
    // sort out the coming alarm and display it
    qsort(alarms, 3, sizeof(Alarm), cmpfunc);
    Serial.print(alarms[0].hour);
    Serial.print(":");
    Serial.println(alarms[0].minute);
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
