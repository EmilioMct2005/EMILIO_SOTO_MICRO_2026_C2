#include <Arduino.h>
#include "config.h"
#include "types.h"
#include "joystick.h"
#include "imu.h"
#include "display.h"
#include "rf_module.h"
#include "mqtt_handler.h"
#include "buttons.h"

// ===== Globals =====
Joystick joystick;
IMU imu;
Display display;
RFModule rf;
Buttons buttons;
LEDs leds;
ControllerState state;

// ===== State =====
bool gyroAsJoystickMode = true;
bool useGyroForRF = true;
unsigned long lastDisplay = 0;
unsigned long lastRF = 0;
unsigned long lastBattery = 0;
unsigned long lastAccelPub = 0;
unsigned long lastGyroPub = 0;
unsigned long lastLoopTime = 0;
uint32_t loopCounter = 0;

// ===== Battery =====
float readBatteryVoltage() {
    uint32_t raw = 0;
    for (int i = 0; i < 16; i++) {
        raw += analogRead(VBAT_PIN);
    }
    raw /= 16;
    float voltage = (raw / (float)ADC_MAX) * 3.3f * VBAT_DIVIDER_RATIO;
    return voltage;
}

uint8_t batteryPercentage(float voltage) {
    float pct = (voltage - VBAT_EMPTY) / (VBAT_FULL - VBAT_EMPTY) * 100.0f;
    return constrain((int)pct, 0, 100);
}

// ===== RF Data Packing =====
void packRFData() {
    state.rfData.joy0X = map(state.joy[0].x, -100, 100, 0, 255);
    state.rfData.joy0Y = map(state.joy[0].y, -100, 100, 0, 255);
    state.rfData.joy1X = map(state.joy[1].x, -100, 100, 0, 255);
    state.rfData.joy1Y = map(state.joy[1].y, -100, 100, 0, 255);

    uint8_t btns = 0;
    for (int i = 0; i < 4; i++) {
        if (state.buttons.trigger[i]) btns |= (1 << i);
        if (state.buttons.side[i]) btns |= (1 << (i + 4));
    }
    state.rfData.buttons = btns;

    if (useGyroForRF) {
        state.rfData.gyroX = map(constrain((int)state.imu.roll, -100, 100), -100, 100, 0, 255);
        state.rfData.gyroY = map(constrain((int)state.imu.pitch, -100, 100), -100, 100, 0, 255);
    }
}

// ===== Calibration =====
void handleCalibration() {
    ButtonEvent ev = buttons.getEvent();

    if (ev.type == ButtonEvent::HOLD && ev.index == 4 && ev.isTrigger == false) {
        Serial.println("[CALIB] Calibration started - hold both SIDE buttons...");
        display.showCalibration();
        leds.setAll(true);

        bool s1 = buttons.getState().side[0];
        bool s2 = buttons.getState().side[1];

        if (s1 && s2) {
            joystick.calibrateBoth();
            Serial.println("[CALIB] Joysticks calibrated!");
            leds.blink(0, 3);
        }
    }
}

// ===== Setup =====
void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.println("=================================");
    Serial.println("  KAKATA RC433 V1 - Controller");
    Serial.println("  ITLA-HUB 2026");
    Serial.println("=================================");

    joystick.begin();
    buttons.begin();
    leds.begin();
    display.begin();

    Serial.println("[INIT] Starting MPU6050...");
    imu.begin();

    Serial.println("[INIT] Starting RF 433MHz...");
    rf.begin();
    rf.enable();
    state.rfEnabled = true;

    Serial.println("[INIT] Connecting WiFi + MQTT...");
    mqttHandler.begin();

    state.wifiConnected = (WiFi.status() == WL_CONNECTED);
    state.mqttConnected = mqttHandler.isConnected();

    lastLoopTime = millis();
    Serial.println("[INIT] System ready!");
    Serial.println();
}

// ===== Main Loop =====
void loop() {
    unsigned long now = millis();

    // === Read inputs ===
    joystick.updateAll();
    buttons.update();
    imu.update();

    // === Gyro as joystick ===
    if (gyroAsJoystickMode) {
        IMUData& imuData = imu.getData();
        float roll = imuData.roll;
        float pitch = imuData.pitch;

        int16_t gx = constrain((int)(roll * 100.0f / 45.0f), -100, 100);
        int16_t gy = constrain((int)(pitch * 100.0f / 45.0f), -100, 100);

        if (abs(gx) > 5 || abs(gy) > 5) {
            state.joy[0].x = (state.joy[0].x + gx) / 2;
            state.joy[0].y = (state.joy[0].y + gy) / 2;
        }
    }

    // === Handle calibration hold (side buttons 0+1 for 3s) ===
    static bool side0Held = false;
    static bool side1Held = false;
    static unsigned long sideHoldStart = 0;

    if (buttons.getState().side[0] && buttons.getState().side[1]) {
        if (!side0Held && !side1Held) {
            side0Held = true;
            side1Held = true;
            sideHoldStart = now;
        } else if (now - sideHoldStart >= CALIB_HOLD_MS) {
            Serial.println("[CALIB] Calibrating joysticks...");
            display.showCalibration();
            leds.blink(0, 3);
            joystick.calibrateBoth();
            Serial.println("[CALIB] Done!");
            side0Held = false;
            side1Held = false;
        }
    } else {
        side0Held = false;
        side1Held = false;
    }

    // === Update state ===
    state.joy[0] = joystick.getState(0);
    state.joy[1] = joystick.getState(1);
    state.buttons = buttons.getState();
    state.imu = imu.getData();
    state.wifiConnected = (WiFi.status() == WL_CONNECTED);
    state.mqttConnected = mqttHandler.isConnected();
    state.battery.voltage = readBatteryVoltage();
    state.battery.percentage = batteryPercentage(state.battery.voltage);
    state.loopCount = loopCounter++;

    if (now - lastLoopTime > 0) {
        state.loopHz = 1000.0f / (now - lastLoopTime);
    }
    lastLoopTime = now;

    // === RF Send ===
    if (now - lastRF >= RF_RATE_MS && state.rfEnabled) {
        packRFData();
        rf.send(state.rfData);
        lastRF = now;
    }

    // === MQTT Publish ===
    if (state.mqttConnected) {
        mqttHandler.loop();
        mqttHandler.publish(state);

        if (now - lastAccelPub >= 1000) {
            mqttHandler.publishAccel(state.imu);
            lastAccelPub = now;
        }
        if (now - lastGyroPub >= 500) {
            mqttHandler.publishGyro(state.imu);
            lastGyroPub = now;
        }
    } else {
        mqttHandler.loop();
    }

    // === LEDs ===
    leds.update(state);

    // === Display ===
    if (now - lastDisplay >= DISPLAY_RATE_MS) {
        display.update(state);
        lastDisplay = now;
    }

    // === Debug Serial ===
    if (loopCounter % 100 == 0) {
        Serial.print("[CTRL] J0:");
        Serial.print(state.joy[0].x);
        Serial.print(",");
        Serial.print(state.joy[0].y);
        Serial.print(" J1:");
        Serial.print(state.joy[1].x);
        Serial.print(",");
        Serial.print(state.joy[1].y);
        Serial.print(" IMU:");
        Serial.print((int)state.imu.roll);
        Serial.print(",");
        Serial.print((int)state.imu.pitch);
        Serial.print(" BAT:");
        Serial.print(state.battery.voltage, 1);
        Serial.print("V ");
        Serial.print(state.battery.percentage);
        Serial.print("% Hz:");
        Serial.print((int)state.loopHz);
        Serial.print(" T:");
        for (int i = 0; i < 4; i++) Serial.print(state.buttons.trigger[i] ? "1" : "0");
        Serial.print(" S:");
        for (int i = 0; i < 4; i++) Serial.print(state.buttons.side[i] ? "1" : "0");
        Serial.println();
    }
}
