#pragma once

#include "config.h"
#include "types.h"
#include <Wire.h>
#include <Adafruit_MPU6050.h>

class IMU {
public:
    void begin();
    void update();
    IMUData& getData();
    void gyroAsJoystick(JoystickState& joyOut, int axis);
    void gyroToRF(RFData& rf);

private:
    Adafruit_MPU6050 _mpu;
    IMUData _data;
    float _gyroBiasX = 0, _gyroBiasY = 0, _gyroBiasZ = 0;
    float _angleX = 0, _angleY = 0, _angleZ = 0;
    unsigned long _lastUpdate = 0;
    void calibrateGyro();
};
