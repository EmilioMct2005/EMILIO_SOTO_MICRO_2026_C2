#pragma once

#include <stdint.h>

struct JoystickState {
    int16_t x;       // -100 to +100
    int16_t y;       // -100 to +100
    bool pressed;
    uint16_t rawX;
    uint16_t rawY;
    uint16_t offsetX; // calibration offsets
    uint16_t offsetY;
    bool calibrated;
};

struct IMUData {
    float accelX;     // m/s^2
    float accelY;
    float accelZ;
    float gyroX;      // degrees/s
    float gyroY;
    float gyroZ;
    float pitch;      // fused angles
    float roll;
    float yaw;
    float temperature; // celsius
    bool available;
};

struct ButtonState {
    bool trigger[4];  // 4 gatillos (front)
    bool side[4];     // 4 botones laterales
    bool joy[2];      // joystick buttons
};

struct ButtonEvent {
    enum Type { NONE, PRESS, RELEASE, HOLD };
    Type type;
    uint8_t index;    // button index
    bool isTrigger;
};

struct RFData {
    uint8_t joy0X;    // mapped 0-255 (128 = center)
    uint8_t joy0Y;
    uint8_t joy1X;
    uint8_t joy1Y;
    uint8_t buttons;  // bitfield: 8 buttons
    uint8_t gyroX;    // mapped 0-255 (128 = center)
    uint8_t gyroY;
    uint8_t checksum;
};

struct BatteryState {
    float voltage;
    uint8_t percentage;
    bool charging;
};

struct ControllerState {
    JoystickState joy[2];
    IMUData imu;
    ButtonState buttons;
    BatteryState battery;
    RFData rfData;
    bool wifiConnected;
    bool mqttConnected;
    bool rfEnabled;
    uint32_t loopCount;
    float loopHz;
};
