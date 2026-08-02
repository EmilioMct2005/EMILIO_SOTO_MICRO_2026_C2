#include "mqtt_handler.h"
#include <ArduinoJson.h>

MQTTHandler mqttHandler;

void MQTTHandler::_callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("[MQTT] Message on: ");
    Serial.println(topic);
}

void MQTTHandler::begin() {
    _mqtt.setClient(_wifi);
    _mqtt.setServer(MQTT_BROKER, MQTT_PORT);
    _mqtt.setCallback(_callback);
    connectWiFi();
}

void MQTTHandler::connectWiFi() {
    Serial.print("[WIFI] Connecting to ");
    Serial.println(WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    int tries = 0;
    while (WiFi.status() != WL_CONNECTED && tries < 30) {
        delay(500);
        Serial.print(".");
        tries++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println();
        Serial.print("[WIFI] Connected! IP: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println();
        Serial.println("[WIFI] Connection failed!");
    }
}

void MQTTHandler::connectMQTT() {
    if (WiFi.status() != WL_CONNECTED) {
        connectWiFi();
        return;
    }

    if (_mqtt.connected()) return;

    Serial.print("[MQTT] Connecting to ");
    Serial.print(MQTT_BROKER);
    Serial.print(":");
    Serial.println(MQTT_PORT);

    if (_mqtt.connect(MQTT_CLIENT_ID)) {
        Serial.println("[MQTT] Connected!");
        _mqtt.subscribe(String(MQTT_TOPIC_BASE "/cmd").c_str());
        _mqtt.subscribe(String(MQTT_TOPIC_BASE "/config").c_str());
    } else {
        Serial.print("[MQTT] Failed, rc=");
        Serial.println(_mqtt.state());
    }
}

void MQTTHandler::loop() {
    if (!_mqtt.connected()) {
        connectMQTT();
    }
    _mqtt.loop();
}

void MQTTHandler::publish(ControllerState& state) {
    if (!_mqtt.connected()) return;

    unsigned long now = millis();
    if (now - _lastPublish < MQTT_RATE_MS) return;
    _lastPublish = now;

    StaticJsonDocument<512> doc;

    JsonObject j0 = doc.createNestedObject("joy0");
    j0["x"] = state.joy[0].x;
    j0["y"] = state.joy[0].y;
    j0["btn"] = state.joy[0].pressed;

    JsonObject j1 = doc.createNestedObject("joy1");
    j1["x"] = state.joy[1].x;
    j1["y"] = state.joy[1].y;
    j1["btn"] = state.joy[1].pressed;

    JsonArray trig = doc.createNestedArray("triggers");
    for (int i = 0; i < 4; i++) trig.add(state.buttons.trigger[i]);

    JsonArray side = doc.createNestedArray("side");
    for (int i = 0; i < 4; i++) side.add(state.buttons.side[i]);

    JsonObject imu = doc.createNestedObject("imu");
    imu["roll"] = round(state.imu.roll * 10) / 10.0;
    imu["pitch"] = round(state.imu.pitch * 10) / 10.0;
    imu["yaw"] = round(state.imu.yaw * 10) / 10.0;

    JsonObject bat = doc.createNestedObject("battery");
    bat["voltage"] = round(state.battery.voltage * 100) / 100.0;
    bat["pct"] = state.battery.percentage;

    JsonObject sys = doc.createNestedObject("sys");
    sys["hz"] = (int)state.loopHz;
    sys["wifi"] = state.wifiConnected;
    sys["mqtt"] = state.mqttConnected;
    sys["rf"] = state.rfEnabled;

    char buf[512];
    serializeJson(doc, buf, sizeof(buf));

    _mqtt.publish(MQTT_TOPIC_BASE "/telemetry", buf);
}

void MQTTHandler::publishAccel(IMUData& imu) {
    if (!_mqtt.connected()) return;

    StaticJsonDocument<256> doc;
    doc["ax"] = round(imu.accelX * 100) / 100.0;
    doc["ay"] = round(imu.accelY * 100) / 100.0;
    doc["az"] = round(imu.accelZ * 100) / 100.0;
    doc["temp"] = round(imu.temperature * 10) / 10.0;

    char buf[256];
    serializeJson(doc, buf, sizeof(buf));
    _mqtt.publish(MQTT_TOPIC_BASE "/accel", buf);
}

void MQTTHandler::publishGyro(IMUData& imu) {
    if (!_mqtt.connected()) return;

    StaticJsonDocument<256> doc;
    doc["gx"] = round(imu.gyroX * 1000) / 1000.0;
    doc["gy"] = round(imu.gyroY * 1000) / 1000.0;
    doc["gz"] = round(imu.gyroZ * 1000) / 1000.0;
    doc["roll"] = round(imu.roll * 10) / 10.0;
    doc["pitch"] = round(imu.pitch * 10) / 10.0;

    char buf[256];
    serializeJson(doc, buf, sizeof(buf));
    _mqtt.publish(MQTT_TOPIC_BASE "/gyro", buf);
}

bool MQTTHandler::isConnected() {
    return _mqtt.connected() && WiFi.status() == WL_CONNECTED;
}
