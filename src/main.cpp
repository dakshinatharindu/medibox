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
const char *settingsTopic = "190622R/settings";
const char *switchBackTopic = "190622R/switch_back";

// Global variables
unsigned long dhtTime;
unsigned long ldrTime;
unsigned long buzzerTime;
bool buzzerState = false;
bool mainSwitch = false;
bool schedule = false;
int days = 0;
int minAngle = 30;
float contrlingFactor = 0.75;
int buzzerDelay = 1000;
int buzzerFrequency = 256;
int buzzerType = 0;
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
void buzzerRing();
void buzzerStop();

//////////////////// ALARM ////////////////////
int cmpfunc(const void *a, const void *b) {
    Alarm *alarm1 = (Alarm *)a;
    Alarm *alarm2 = (Alarm *)b;
    int compare1 =
        alarm1->isOn * 2000 + (24 - alarm1->hour) * 60 + (60 - alarm1->minute);
    int compare2 =
        alarm2->isOn * 2000 + (24 - alarm2->hour) * 60 + (60 - alarm2->minute);
    return compare2 - compare1;
}

void checkAlarm() {
    DateTime now = rtc.now();
    static uint8_t lastMinute = 0;

    if (now.minute() != lastMinute) {
        lastMinute = now.minute();
        for (int i = 0; i < 3; i++) {
            if (alarms[i].isOn && alarms[i].hour == now.hour() &&
                alarms[i].minute == now.minute()) {
                mainSwitch = true;
                mqttClient.publish(switchBackTopic, "1");
            }
        }
    }
}

//////////////////// MQTT ////////////////////
void mqttCallback(char *topic, byte *payload, unsigned int length) {
    char payloadCharArr[length];

    for (int i = 0; i < length; i++) {
        payloadCharArr[i] = (char)payload[i];
    }
    String payloadStr = String(payloadCharArr).substring(0, length);
    if (strcmp(topic, mainBuzzerTopic) == 0) {
        mainSwitch = payloadStr.toInt();
        if (mainSwitch == 0) lcdShowAlarm();
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

        days = payloadStr.substring(16, 18).toInt();
        if (mainSwitch == 0) lcdShowAlarm();

    } else if (strcmp(topic, minAngleTopic) == 0) {
        minAngle = payloadStr.toInt();
    } else if (strcmp(topic, contrlingFactorTopic) == 0) {
        contrlingFactor = payloadStr.toFloat();
    } else if (strcmp(topic, settingsTopic) == 0) {
        buzzerDelay = payloadStr.substring(0, 5).toInt();
        buzzerFrequency = payloadStr.substring(5, 10).toInt();
        buzzerType = payloadStr.substring(10, 11).toInt();
    }
}

void mqttInit() {
    mqttClient.setServer(mqqttBroker, mqttPort);
    mqttClient.setCallback(mqttCallback);
    WiFi.begin(SSID, PASSWORD);

    lcd.setCursor(0, 0);
    lcd.print("Connecting ");
    while (WiFi.status() != WL_CONNECTED) {
        lcd.print(".");
        delay(500);
    }
    lcd.setCursor(0, 1);
    lcd.print("Connected to WiFI");
    lcd.setCursor(0, 2);
    lcd.print("SSID: ");
    lcd.print(SSID);
    lcd.setCursor(0, 3);
    lcd.print("IP: ");
    lcd.print(WiFi.localIP());
    delay(1000);
    lcd.clear();
}

void mqttLoop() {
    while (!mqttClient.connected()) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Connecting to MQTT..");
        if (mqttClient.connect(mqttClientID)) {
            lcd.setCursor(0, 1);
            lcd.print("Connected to MQTT");
            mqttClient.subscribe(mainBuzzerTopic);
            mqttClient.subscribe(alarmsTopic);
            mqttClient.subscribe(minAngleTopic);
            mqttClient.subscribe(contrlingFactorTopic);
            mqttClient.subscribe(settingsTopic);
            delay(1000);
            lcd.clear();
        } else {
            lcd.setCursor(0, 1);
            lcd.print("failed, rc=");
            lcd.print(mqttClient.state());
            lcd.setCursor(0, 2);
            lcd.println("TRY AGAIN IN");
            lcd.setCursor(0, 3);
            lcd.print("    5 SECONDS");
            delay(5000);
            lcd.clear();
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
    lcd.print("********************");
    lcd.setCursor(0, 1);
    lcd.print("       MEDIBOX      ");
    lcd.setCursor(0, 2);
    lcd.print("YOUR PERSONAL DOCTOR");
    lcd.setCursor(0, 3);
    lcd.print("********************");
    delay(1000);
    lcd.clear();
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
    DateTime now = rtc.now();
    lcd.setCursor(0, 2);
    lcd.print("  UPCOMING SCHEDULE ");
    lcd.setCursor(0, 3);
    // sort out the coming alarm and display it
    qsort(alarms, 3, sizeof(Alarm), cmpfunc);
    if (schedule) {
        if (alarms[0].isOn && alarms[0].hour > now.hour() ||
            (alarms[0].hour == now.hour() && alarms[0].minute > now.minute())) {
            lcd.print("        ");
            if (alarms[0].hour < 10) lcd.print('0');
            lcd.print(alarms[0].hour, DEC);
            lcd.print(':');
            if (alarms[0].minute < 10) lcd.print('0');
            lcd.print(alarms[0].minute, DEC);
            lcd.print("       ");
        } else if (alarms[1].isOn && alarms[1].hour > now.hour() ||
                   (alarms[1].hour == now.hour() &&
                    alarms[1].minute > now.minute())) {
            lcd.print("        ");
            if (alarms[1].hour < 10) lcd.print('0');
            lcd.print(alarms[1].hour, DEC);
            lcd.print(':');
            if (alarms[1].minute < 10) lcd.print('0');
            lcd.print(alarms[1].minute, DEC);
            lcd.print("       ");
        } else if (alarms[2].isOn && alarms[2].hour > now.hour() ||
                   (alarms[2].hour == now.hour() &&
                    alarms[2].minute > now.minute())) {
            lcd.print("        ");
            if (alarms[2].hour < 10) lcd.print('0');
            lcd.print(alarms[2].hour, DEC);
            lcd.print(':');
            if (alarms[2].minute < 10) lcd.print('0');
            lcd.print(alarms[2].minute, DEC);
            lcd.print("       ");
        } else if (alarms[0].isOn && days > 0) {
            lcd.print("        ");
            if (alarms[0].hour < 10) lcd.print('0');
            lcd.print(alarms[0].hour, DEC);
            lcd.print(':');
            if (alarms[0].minute < 10) lcd.print('0');
            lcd.print(alarms[0].minute, DEC);
            lcd.print("       ");
        } else {
            lcd.print("     NO SCHEDULE    ");
        }
    } else {
        lcd.print("     NO SCHEDULE    ");
    }
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
    buzzerTime = millis();
}

void buzzerLoop() {
    if (mainSwitch) {
        if (buzzerType == 1) {
            ledcWriteTone(BUZZER_CHANNEL, buzzerFrequency);
            lcd.setCursor(0, 2);
            lcd.print("  IT'S TIME TO TAKE ");
            lcd.setCursor(0, 3);
            lcd.print("     YOUR MEDICINE  ");
        } else {
            if (millis() - buzzerTime > buzzerDelay) {
                buzzerTime = millis();
                buzzerState = !buzzerState;
                if (buzzerState) {
                    ledcWriteTone(BUZZER_CHANNEL, buzzerFrequency);
                    lcd.setCursor(0, 2);
                    lcd.print("  IT'S TIME TO TAKE ");
                    lcd.setCursor(0, 3);
                    lcd.print("     YOUR MEDICINE  ");
                } else {
                    ledcWrite(BUZZER_CHANNEL, 0);
                    lcd.setCursor(0, 2);
                    lcd.print("                    ");
                    lcd.setCursor(0, 3);
                    lcd.print("                    ");
                }
            }
        }
    } else {
        ledcWrite(BUZZER_CHANNEL, 0);
    }
}

//////////////////// RTC ////////////////////
void rtcInit() {
    if (!rtc.begin()) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Couldn't find RTC");
        delay(1000);
        lcd.clear();
        abort();
    }
}

//////////////////// SETUP ////////////////////
void setup() {
    // Init Serial
    // Serial.begin(115200);
    // delay(10);

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
    checkAlarm();
    buzzerLoop();
}
