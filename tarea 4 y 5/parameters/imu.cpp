#include "imu.h"

void IMU::begin() {
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, 400000);

    _data = {};

    if (!_mpu.begin(MPU6050_ADDR, &Wire)) {
        Serial.println("[IMU] MPU6050 not found!");
        _data.available = false;
        return;
    }

    _data.available = true;
    _mpu.setAccelerometerRange(MPU6050_RANGE_4_G);
    _mpu.setGyroRange(MPU6050_RANGE_250_DEG);
    _mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

    calibrateGyro();
    _lastUpdate = millis();
    Serial.println("[IMU] MPU6050 ready");
}

void IMU::calibrateGyro() {
    const int samples = 200;
    float sx = 0, sy = 0, sz = 0;
    for (int i = 0; i < samples; i++) {
        sensors_event_t a, g, t;
        _mpu.getEvent(&a, &g, &t);
        sx += g.gyro.x;
        sy += g.gyro.y;
        sz += g.gyro.z;
        delay(5);
    }
    _gyroBiasX = sx / samples;
    _gyroBiasY = sy / samples;
    _gyroBiasZ = sz / samples;
    _angleX = 0;
    _angleY = 0;
    _angleZ = 0;
}

void IMU::update() {
    if (!_data.available) return;

    sensors_event_t accel, gyro, temp;
    _mpu.getEvent(&accel, &gyro, &temp);

    _data.accelX = accel.acceleration.x;
    _data.accelY = accel.acceleration.y;
    _data.accelZ = accel.acceleration.z;
    _data.gyroX = gyro.gyro.x;
    _data.gyroY = gyro.gyro.y;
    _data.gyroZ = gyro.gyro.z;
    _data.temperature = temp.temperature;

    unsigned long now = millis();
    float dt = (now - _lastUpdate) / 1000.0f;
    _lastUpdate = now;

    if (dt > 0 && dt < 0.1f) {
        float gx = gyro.gyro.x - _gyroBiasX;
        float gy = gyro.gyro.y - _gyroBiasY;
        float gz = gyro.gyro.z - _gyroBiasZ;

        _angleX += gx * dt * 57.2958f;
        _angleY += gy * dt * 57.2958f;
        _angleZ += gz * dt * 57.2958f;

        float accelAngleX = atan2(_data.accelY, _data.accelZ) * 57.2958f;
        float accelAngleY = atan2(-_data.accelX, sqrt(_data.accelY * _data.accelY + _data.accelZ * _data.accelZ)) * 57.2958f;

        _angleX = 0.98f * _angleX + 0.02f * accelAngleX;
        _angleY = 0.98f * _angleY + 0.02f * accelAngleY;
    }

    _data.pitch = constrain(_angleX, -45.0f, 45.0f);
    _data.roll = constrain(_angleY, -45.0f, 45.0f);
    _data.yaw = _angleZ;
}

void IMU::gyroAsJoystick(JoystickState& joyOut, int axis) {
    if (!_data.available) return;
    float val;
    if (axis == 0) {
        val = _data.roll;   // left/right
    } else {
        val = _data.pitch;  // up/down
    }
    joyOut.x = (axis == 0) ? constrain((int)val * 100 / 45, -100, 100) : joyOut.x;
    joyOut.y = (axis == 1) ? constrain((int)val * 100 / 45, -100, 100) : joyOut.y;
}

void IMU::gyroToRF(RFData& rf) {
    if (!_data.available) {
        rf.gyroX = 128;
        rf.gyroY = 128;
        return;
    }
    rf.gyroX = map(constrain((int)_data.roll, -100, 100), -100, 100, 0, 255);
    rf.gyroY = map(constrain((int)_data.pitch, -100, 100), -100, 100, 0, 255);
}

IMUData& IMU::getData() {
    return _data;
}
