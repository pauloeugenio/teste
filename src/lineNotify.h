
#ifndef LINE_NOTIFY_H
#define LINE_NOTIFY_H

#include <Arduino.h>
#include <WiFiClientSecure.h>

#include "configs.h"

void Line_Notify(String message) {
    if (!ENABLE_LINE_NOTIFY)
        return;

    Serial.println("Send Line-Notify");
    WiFiClientSecure client;
    client.setInsecure();
    if (!client.connect("notify-api.line.me", 443)) {
        Serial.println("connection failed");
        return;
    }

    String req = "";
    req += "POST /api/notify HTTP/1.1\r\n";
    req += "Host: notify-api.line.me\r\n";
    req += "Authorization: Bearer " + String(LINE_TOKEN) + "\r\n";
    req += "Cache-Control: no-cache\r\n";
    req += "User-Agent: ESP32\r\n";
    req += "Content-Type: application/x-www-form-urlencoded\r\n";
    req += "Content-Length: " + String(String("message=" + message).length()) + "\r\n";
    req += "\r\n";
    req += "message=" + message;

    if (ENABLE_DEBUG_MODE)
        Serial.println(req);

    client.print(req);
    delay(20);

    Serial.println("-------------");
    while (client.connected()) {
        String line = client.readStringUntil('\n');
        if (line == "\r") {
            break;
        }
        if (ENABLE_DEBUG_MODE)
            Serial.println(line);
    }
    Serial.println("-------------");
}

#endif