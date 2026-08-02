#include "joystick.h"

static const int JOY_PINS[2][2] = {
    {JOY0_X_PIN, JOY0_Y_PIN},
    {JOY1_X_PIN, JOY1_Y_PIN}
};
static const int JOY_BTN_PINS[2] = {JOY0_BTN_PIN, JOY1_BTN_PIN};

void Joystick::begin() {
    analogReadResolution(ADC_RESOLUTION);
    for (int j = 0; j < 2; j++) {
        pinMode(JOY_BTN_PINS[j], INPUT_PULLUP);
        _joy[j] = {};
        _joy[j].offsetX = ADC_CENTER;
        _joy[j].offsetY = ADC_CENTER;
        _joy[j].calibrated = false;
    }
}

void Joystick::setDeadzone(uint16_t dz) {
    _deadzone = dz;
}

int16_t Joystick::mapToRange(uint16_t raw, uint16_t center) {
    int32_t diff = (int32_t)raw - (int32_t)center;
    if (abs(diff) < _deadzone) return 0;
    if (diff > 0) {
        return map(diff, 0, ADC_MAX - center, 0, 100);
    } else {
        return map(diff, 0, (int32_t)0 - center, 0, -100);
    }
}

void Joystick::update(int joyIdx) {
    if (joyIdx < 0 || joyIdx > 1) return;
    JoystickState& j = _joy[joyIdx];

    j.rawX = analogRead(JOY_PINS[joyIdx][0]);
    j.rawY = analogRead(JOY_PINS[joyIdx][1]);
    j.x = mapToRange(j.rawX, j.offsetX);
    j.y = mapToRange(j.rawY, j.offsetY);
    j.pressed = (digitalRead(JOY_BTN_PINS[joyIdx]) == LOW);
}

void Joystick::updateAll() {
    update(0);
    update(1);
}

void Joystick::calibrate(int joyIdx) {
    if (joyIdx < 0 || joyIdx > 1) return;
    uint32_t sumX = 0, sumY = 0;
    const int samples = 50;
    for (int i = 0; i < samples; i++) {
        sumX += analogRead(JOY_PINS[joyIdx][0]);
        sumY += analogRead(JOY_PINS[joyIdx][1]);
        delay(10);
    }
    _joy[joyIdx].offsetX = sumX / samples;
    _joy[joyIdx].offsetY = sumY / samples;
    _joy[joyIdx].calibrated = true;
}

void Joystick::calibrateBoth() {
    calibrate(0);
    calibrate(1);
}

JoystickState& Joystick::getState(int idx) {
    return _joy[idx];
}
