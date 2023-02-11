
#ifndef MAKE_RESPONSE_H
#define MAKE_RESPONSE_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <SocketIOclient.h>

#include "configs.h"
#include "lineNotify.h"
#include "utility.h"

String createResponse(SocketIOclient &webSocket,
                      float DC_VOLTAGE, float DC_CURRENT, float DC_POWER, float DC_ENERGY,
                      float AC_VOLTAGE, float AC_CURRENT, float AC_POWER, float AC_ENERGY, float AC_FREQUENCY, float AC_PF,
                      float HUMIDITY, float TEMPERATURE) {
    DynamicJsonDocument root(1024);
    JsonArray array = root.to<JsonArray>();
    array.add(SOCKETIO_CHANNEL);

    JsonObject doc = array.createNestedObject();

    doc["deviceId"] = getChipId();
    doc["deviceName"] = DEVICE_NAME;
    doc["time"] = NowString();

    JsonObject data = doc.createNestedObject("sensor");
    // DHT11
    data["humidity"] = HUMIDITY;
    data["temperature"] = TEMPERATURE;

    JsonObject dc = data.createNestedObject("dc");
    // PZEM-017
    dc["voltage_usage"] = DC_VOLTAGE;
    dc["current_usage"] = DC_CURRENT;
    dc["active_power"] = DC_POWER;
    dc["active_energy"] = DC_ENERGY;

    JsonObject ac = data.createNestedObject("ac");
    // PZEM-004T
    ac["voltage_usage"] = AC_VOLTAGE;
    ac["current_usage"] = AC_CURRENT;
    ac["active_power"] = AC_POWER;
    ac["active_energy"] = AC_ENERGY;
    ac["frequency"] = AC_FREQUENCY;
    ac["pf"] = AC_PF;

    JsonObject deviceState = doc.createNestedObject("deviceState");

    //Active Low
    deviceState["INVERTER"] = String((digitalRead(INVERTER) == LOW) ? "ON" : "OFF");
    deviceState["COOLING_FAN"] = String((digitalRead(COOLING_FAN) == LOW) ? "ON" : "OFF");
    deviceState["LIGHT"] = String((digitalRead(LIGHT) == LOW) ? "ON" : "OFF");
    deviceState["SPOTLIGHT"] = String((digitalRead(SPOTLIGHT) == LOW) ? "ON" : "OFF");
    //Active High
    deviceState["POWER_BACKUP"] = String((digitalRead(POWER_BACKUP) == LOW) ? "OFF" : "ON");
    deviceState["FIRMWARE_VERSION"] = String(FIRMWARE_VERSION);
    deviceState["FIRMWARE_LASTUPDATE"] = FIRMWARE_LASTUPDATE;
    deviceState["IP_ADDRESS"] = WiFi.localIP().toString();

    String output;
    serializeJsonPretty(root, output);

    //Publish to socket.io server
    if (ENABLE_SOCKETIO)
        webSocket.sendEVENT(output.c_str());

    if (ENABLE_DEBUG_MODE)
        Serial.print(output);

    return output;
}

#endif