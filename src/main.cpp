/*

  # Author : Watchara Pongsri
  # [github/X-c0d3] https://github.com/X-c0d3/
  # Web Site: https://wwww.rockdevper.com

*/

#include <Arduino.h>
#include <ArduinoJson.h>
#include <SocketIOclient.h>
#include <WiFiClientSecure.h>
#include <arduino-timer.h>
#include <string.h>

#include "configs.h"
#include "firmware.h"
#include "makeResponse.h"
#include "utility.h"
#include "wifiMan.h"

SocketIOclient webSocket;

auto timer = timer_create_default();  // create a timer with default settings
Timer<> default_timer;                // save as above

void handleRelaySwitch() {
    //ACTIVE LOW
    pinMode(INVERTER, OUTPUT);
    digitalWrite(INVERTER, HIGH);
    pinMode(COOLING_FAN, OUTPUT);
    digitalWrite(COOLING_FAN, HIGH);
    pinMode(LIGHT, OUTPUT);
    digitalWrite(LIGHT, HIGH);
    pinMode(SPOTLIGHT, OUTPUT);
    digitalWrite(SPOTLIGHT, HIGH);
    //ACTIVE HIGH
    pinMode(POWER_BACKUP, OUTPUT);
    digitalWrite(POWER_BACKUP, LOW);
}

unsigned long interval = 300;      // the time we need to wait
unsigned long previousMillis = 0;  // millis() returns an unsigned long.
hw_timer_t *watchdogTimer = NULL;

void event(uint8_t *payload, size_t length) {
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, payload);
    if (error)
        return;

    String action = doc[1]["action"];
    unsigned long currentMillis = millis();
    if (action != "null") {
        String state = doc[1]["payload"]["state"];
        String messageInfo = doc[1]["payload"]["messageInfo"];
        bool isAuto = doc[1]["payload"]["isAuto"];

        if ((unsigned long)(currentMillis - previousMillis) >= interval) {
            if (ENABLE_DEBUG_MODE) {
                Serial.printf("=====>: %s\n", payload);
            }
            //actionCommand(webSocket, action, state, messageInfo, isAuto, true);

            // save the "current" time
            previousMillis = currentMillis;
        }
    }
}

String jsonResult;
bool readData(void *) {
    if (firmwareUpgradeProgress > 0)  //Kill task when firmware start upgrading
        return true;

    float DC_VOLTAGE, DC_CURRENT, DC_POWER, DC_ENERGY, AC_VOLTAGE, AC_CURRENT, AC_POWER, AC_ENERGY, AC_FREQUENCY, AC_PF, HUMIDITY, TEMPERATURE;
    // For testing
    if (ENABLE_TEST_MODE) {
        DC_VOLTAGE = random(350, 400);
        DC_CURRENT = random(5, 7);
        DC_POWER = random(90, 200);
        HUMIDITY = random(50, 100);
        TEMPERATURE = random(20, 50);

        AC_VOLTAGE = random(200, 250);
        AC_CURRENT = random(8, 15);
        AC_POWER = random(300, 400);
        AC_FREQUENCY = random(49, 54);
    }

    if (ENABLE_DEBUG_MODE) {
        Serial.println("########### PZEM-017 ###############");
        Serial.println("PZEM-017 V: " + String(DC_VOLTAGE));
        Serial.println("PZEM-017 A: " + String(DC_CURRENT));
        Serial.println("PZEM-017 W:" + String(DC_POWER));
        Serial.println("PZEM-017 E: " + String(DC_ENERGY));

        Serial.println("########### PZEM-014 ###############");
        Serial.println("PZEM-004T VOLTAGE: " + String(AC_VOLTAGE));
        Serial.println("PZEM-004T CURRENT: " + String(AC_CURRENT));
        Serial.println("PZEM-004T POWER:" + String(AC_POWER));
        Serial.println("PZEM-004T ENERGY: " + String(AC_ENERGY));
        Serial.println("PZEM-004T FREQUENCY: " + String(AC_FREQUENCY));
        Serial.println("PZEM-004T PF: " + String(AC_PF));
        Serial.println("FIRMWARE_VERSION: " + String(FIRMWARE_VERSION));
    }

    jsonResult = createResponse(webSocket, DC_VOLTAGE, DC_CURRENT, DC_POWER, DC_ENERGY,            //DC
                                AC_VOLTAGE, AC_CURRENT, AC_POWER, AC_ENERGY, AC_FREQUENCY, AC_PF,  //AC
                                HUMIDITY, TEMPERATURE);

    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));

    timerWrite(watchdogTimer, 0);  // reset timer feed dog

    return true;
}

void interruptReboot() {
    //Prevent reboot when firmware upgrading
    if (firmwareUpgradeProgress == 0) {
        ets_printf("reboot (Watch Dog)\n");
        esp_restart();
    }
}

void setupWatchDog() {
    Serial.print("Setting timer in setup");
    watchdogTimer = timerBegin(0, 80, true);
    //timer 0 divisor 80
    timerAlarmWrite(watchdogTimer, 10000000, false);  // 10 sec set time in uS must be fed within this time or reboot
    timerAttachInterrupt(watchdogTimer, &interruptReboot, true);
    timerAlarmEnable(watchdogTimer);  // enable interrupt
}

void hexdump(const void *mem, uint32_t len, uint8_t cols = 16) {
    const uint8_t *src = (const uint8_t *)mem;
    Serial.printf("\n[HEXDUMP] Address: 0x%08X len: 0x%X (%d)", (ptrdiff_t)src, len, len);
    for (uint32_t i = 0; i < len; i++) {
        if (i % cols == 0) {
            Serial.printf("\n[0x%08X] 0x%08X: ", (ptrdiff_t)src, i);
        }
        Serial.printf("%02X ", *src);
        src++;
    }
    Serial.printf("\n");
}

void socketIOEvent(socketIOmessageType_t type, uint8_t *payload, size_t length) {
    switch (type) {
        case sIOtype_DISCONNECT:
            Serial.printf("[IOc] Disconnected!\n");
            break;
        case sIOtype_CONNECT:
            Serial.printf("[IOc] Connected to url: %s\n", payload);

            // join default namespace (no auto join in Socket.IO V3)
            webSocket.send(sIOtype_CONNECT, "/");
            break;
        case sIOtype_EVENT:
            //Serial.printf("[IOc] get event: %s\n", payload);
            event(payload, length);
            break;
        case sIOtype_ACK:
            Serial.printf("[IOc] get ack: %u\n", length);
            hexdump(payload, length);
            break;
        case sIOtype_ERROR:
            Serial.printf("[IOc] get error: %u\n", length);
            hexdump(payload, length);
            break;
        case sIOtype_BINARY_EVENT:
            Serial.printf("[IOc] get binary: %u\n", length);
            hexdump(payload, length);
            break;
        case sIOtype_BINARY_ACK:
            Serial.printf("[IOc] get binary ack: %u\n", length);
            hexdump(payload, length);
            break;
    }
}

void setup() {
    Serial.begin(DEFAULT_BAUD_RATE);

    delay(2000);

    //wifiMan();
    setup_Wifi();

    if (WiFi.status() == WL_CONNECTED) {
        pinMode(LED_BUILTIN, OUTPUT);

        setupTimeZone();

        handleRelaySwitch();

        webSocket.begin(SOCKETIO_HOST, String(SOCKETIO_PORT).toInt());
        webSocket.onEvent(socketIOEvent);
        webSocket.setReconnectInterval(5000);

        timer.every(DEFAULT_INTERVAL, readData);

        if (ENABLE_FIRMWARE_AUTOUPDATE)
            timer.every(CHECK_FIRMWARE_INTERVAL, updateFirmware);

        setupWatchDog();
    }
}

void loop() {
    webSocket.loop();
    timer.tick();
}