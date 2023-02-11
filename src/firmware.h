#ifndef FIRMWARE_H
#define FIRMWARE_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFiClientSecure.h>

#include "configs.h"
#include "lineNotify.h"

int last = 0;
int firmwareUpgradeProgress = 0;
void OnProgress(int progress, int totalt) {
    firmwareUpgradeProgress = (100 * progress) / totalt;
    if (last != firmwareUpgradeProgress && firmwareUpgradeProgress % 10 == 0) {
        //print every 10%
        Line_Notify("Current Version is " + String(FIRMWARE_VERSION) + " Updating ===>>> " + firmwareUpgradeProgress + "%");
        Serial.println("############## Current Version is " + String(FIRMWARE_VERSION) + " Updating =====>>> " + firmwareUpgradeProgress + "%");
    }
    last = firmwareUpgradeProgress;
}

bool updateFirmware(void *) {
    Serial.println("Checking for firmware updates.");
    Serial.println("Current Version: " + String(FIRMWARE_VERSION));

    HTTPClient http;
    WiFiClientSecure client;
    client.setInsecure();
    client.setTimeout(30000);

    http.begin(client, FIRMWARE_SERVER);

    int httpCode = http.GET();
    if (httpCode == 200) {
        //DynamicJsonBuffer jsonBuffer;
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, http.getString());
        //JsonObject &root = jsonBuffer.parseObject(http.getString());
        if (error)
            return false;

        String newVersion = "";
        String firmwareUrl = "";
        String deviceName = "";
        for (int i = 0; i < (int)doc["devices"].size(); i++) {
            if (DEVICE_NAME == doc["devices"][i]["deviceName"]) {
                deviceName = doc["devices"][i]["deviceName"].as<String>();
                newVersion = doc["devices"][i]["version"].as<String>();
                firmwareUrl = doc["devices"][i]["firmware"].as<String>();
                break;
            }
        }
        if (newVersion == "") return false;
        Serial.println("Device Name: " + deviceName);
        Serial.println("Firmware URL: " + firmwareUrl);
        Serial.println("Next version: " + newVersion);

        String currentVersion = String(FIRMWARE_VERSION);
        currentVersion.replace(".", "");
        String nextVersion = newVersion;
        nextVersion.replace(".", "");

        if (nextVersion.toDouble() > currentVersion.toDouble()) {
            Serial.println("##### Found new version: " + String(newVersion));
            Serial.println("##### Current version: " + String(FIRMWARE_VERSION));
            Update.onProgress(OnProgress);

            Serial.println("##### Start updating... ");
            Line_Notify("Device Firmware Upgrading... version:" + String(newVersion));
            t_httpUpdate_return resF = httpUpdate.update(client, firmwareUrl, String(FIRMWARE_VERSION));
            switch (resF) {
                case HTTP_UPDATE_FAILED:
                    Serial.printf("##### Update faild! (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
                    break;
                case HTTP_UPDATE_NO_UPDATES:
                    Serial.println("##### No new update available");
                    break;
                // We can't see this, because of reset chip after update OK
                case HTTP_UPDATE_OK:
                    Serial.println("##### Update success");
                    Serial.println("##### Rebooting...");
                    break;

                default:
                    break;
            }
        }
    }
    return true;
}

#endif