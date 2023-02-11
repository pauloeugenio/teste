#ifndef CONFIGS_H
#define CONFIGS_H

// Configuration
#define DEVICE_NAME "SolarPower"
#define WIFI_SSID "#{WIFI_SSID}#"
#define WIFI_PASSWORD "#{WIFI_PASSWORD}#"
#define SOCKETIO_HOST "#{SOCKETIO_HOST}#"
#define SOCKETIO_PORT "#{SOCKETIO_PORT}#"
#define SOCKETIO_CHANNEL "#{SOCKETIO_CHANNEL}#"

// Line config
#define LINE_TOKEN "#{LINE_TOKEN}#"

#define FIRMWARE_VERSION "#{FIRMWARE_VERSION}#"
#define FIRMWARE_LASTUPDATE "#{FIRMWARE_LASTUPDATE}#"
#define FIRMWARE_SERVER "#{FIRMWARE_SERVER}#"

#define ENABLE_TEST_MODE false  // ###############
#define ENABLE_DEBUG_MODE true
#define ENABLE_LINE_NOTIFY true
#define ENABLE_SOCKETIO true
#define ENABLE_FIRMWARE_AUTOUPDATE true
#define DEFAULT_BAUD_RATE 115200
#define DEFAULT_INTERVAL 2000
#define CHECK_FIRMWARE_INTERVAL 30000

// Relay Switch (Active : Low)
#define COOLING_FAN 33  //R1
#define INVERTER 25     //R2
#define LIGHT 26        //R3
#define SPOTLIGHT 27    //R4

// Relay Switch (Active : High)
#define POWER_BACKUP 14
#define SW2 12
#define SW3 13

#endif
