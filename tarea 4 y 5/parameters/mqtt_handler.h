#pragma once

#include "config.h"
#include "types.h"
#include <WiFi.h>
#include <PubSubClient.h>

class MQTTHandler {
public:
    void begin();
    void loop();
    void publish(ControllerState& state);
    void publishAccel(IMUData& imu);
    void publishGyro(IMUData& imu);
    bool isConnected();

private:
    WiFiClient _wifi;
    PubSubClient _mqtt;
    void connectWiFi();
    void connectMQTT();
    void callback(char* topic, byte* payload, unsigned int length);
    static void _callback(char* topic, byte* payload, unsigned int length);
    uint32_t _lastPublish = 0;
};

extern MQTTHandler mqttHandler;
