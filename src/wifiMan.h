#ifndef WIFIMAN_H
#define WIFIMAN_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <DNSServer.h>
#include <FS.h>
#include <WiFi.h>
#include <WiFiManager.h>

#ifdef ESP32
#include <SPIFFS.h>
#endif
#include "configs.h"

bool shouldSaveConfig = true;
void saveConfigCallback() {
    Serial.println("Should save config");
    shouldSaveConfig = true;
}

char socketio_server[40];
char socketio_port[6];
char line_api_key[60];
char firebase_host[60];
char firebase_api_key[60];

WiFiManager wifiManager;
void wifiMan() {
    Serial.println("mounting FS...");
    SPIFFS.begin(true);

    if (SPIFFS.begin()) {
        Serial.println("mounted file system");
        if (SPIFFS.exists("/config.json")) {
            //file exists, reading and loading
            Serial.println("reading config file");
            File configFile = SPIFFS.open("/config.json", "r");
            if (configFile) {
                Serial.println("opened config file");
                size_t size = configFile.size();
                // Allocate a buffer to store contents of the file.
                std::unique_ptr<char[]> buf(new char[size]);

                configFile.readBytes(buf.get(), size);
                DynamicJsonDocument json(1024);
                DeserializationError error = deserializeJson(json, buf.get());
                if (error)
                    return;

                serializeJson(json, Serial);
                if (!error) {
                    Serial.println("\nparsed json");

                    strcpy(socketio_server, json["socketIoHost"]);
                    strcpy(socketio_port, json["socketIoPort"]);
                    strcpy(line_api_key, json["lineApiKey"]);
                    strcpy(firebase_host, json["firebaseHost"]);
                    strcpy(firebase_api_key, json["firebaseApiKey"]);

                } else {
                    Serial.println("failed to load json config");
                }
            }
        }
    } else {
        Serial.println("failed to mount FS");
    }

    wifiManager.setSaveConfigCallback(saveConfigCallback);
    //wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

    WiFiManagerParameter socketIoHostInput("socketIoHost", "SocketIO Server", socketio_server, 40);
    WiFiManagerParameter socketIoPortInput("socketIoPort", "SocketIO Port", socketio_port, 6);
    WiFiManagerParameter lineApiKeyInput("lineApiKey", "Line API Token", line_api_key, 60);
    WiFiManagerParameter firebaseHostInput("firebaseHost", "FireBase Host", firebase_host, 60);
    WiFiManagerParameter firebaseApiKeyInput("firebaseKey", "FireBase Api Key", firebase_api_key, 60);

    wifiManager.addParameter(&socketIoHostInput);
    wifiManager.addParameter(&socketIoPortInput);
    wifiManager.addParameter(&lineApiKeyInput);
    wifiManager.addParameter(&firebaseHostInput);
    wifiManager.addParameter(&firebaseApiKeyInput);

    if (!wifiManager.autoConnect("SOLAR_POWER", "1234567890")) {
        Serial.println("failed to connect and hit timeout");
        delay(3000);
        //reset and try again, or maybe put it to deep sleep
        ESP.restart();
        delay(5000);
    }

    // always start configportal for a little while
    // wifiManager.setConfigPortalTimeout(60);
    // wifiManager.startConfigPortal();

    Serial.println("connected.. :)");

    //read updated parameters
    strcpy(socketio_server, socketIoHostInput.getValue());
    strcpy(socketio_port, socketIoPortInput.getValue());
    strcpy(line_api_key, lineApiKeyInput.getValue());
    strcpy(firebase_host, firebaseHostInput.getValue());
    strcpy(firebase_api_key, firebaseApiKeyInput.getValue());

    //save the custom parameters to FS
    if (shouldSaveConfig) {
        Serial.println("saving config");
        DynamicJsonDocument json(1024);
        json["socketIoHost"] = socketio_server;
        json["socketIoPort"] = socketio_port;
        json["lineApiKey"] = line_api_key;
        json["firebaseHost"] = firebase_host;
        json["firebaseApiKey"] = firebase_api_key;

        File configFile = SPIFFS.open("/config.json", "w");
        if (!configFile) {
            Serial.println("failed to open config file for writing");
        }

        serializeJson(json, Serial);
        serializeJson(json, configFile);
        configFile.close();
        //end save
        shouldSaveConfig = false;
    }

    Serial.println("[+] SocketIO Server: " + String(socketio_server));
    Serial.println("[+] SocketIO Port: " + String(socketio_port));
    Serial.println("[+] Line API Token: " + String(line_api_key));
    Serial.println("[+] FireBase Host: " + String(firebase_host));
    Serial.println("[+] FireBase API Token: " + String(firebase_api_key));

    // WiFi.disconnect(true);  //erases store credentially
    // SPIFFS.format();        //erases stored values

    Serial.println("WIFI Config is Done");
    Serial.println("local ip");
    Serial.println(WiFi.localIP());
    Serial.println(WiFi.gatewayIP());
    Serial.println(WiFi.subnetMask());
    Serial.println("###########################################");
}

void wifiReset() {
    Serial.println("Erase settings and restart ...");
    delay(1000);

    WiFi.disconnect(true);  //erases store credentially
    wifiManager.resetSettings();
    SPIFFS.format();  //erases stored values
    ESP.restart();
}

void setup_Wifi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    if (WiFi.getMode() & WIFI_AP) {
        WiFi.softAPdisconnect(true);
    }

    Serial.println();
    Serial.println("WIFI Connecting");

    int i = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(String(i) + ",");
        if (i == 40)
            ESP.restart();

        i++;
    }

    Serial.println();
    Serial.print("WIFI Connected ");
    String ip = WiFi.localIP().toString();
    Serial.println(ip.c_str());
    Serial.println("Socket.io Server: ");
    Serial.print(SOCKETIO_HOST);
    Serial.println();
}

#endif