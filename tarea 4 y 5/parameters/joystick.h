#pragma once

#include "config.h"
#include "types.h"

class Joystick {
public:
    void begin();
    void calibrateBoth();
    void calibrate(int joyIdx);
    void update(int joyIdx);
    void updateAll();
    JoystickState& getState(int idx);
    void setDeadzone(uint16_t dz);

private:
    JoystickState _joy[2];
    uint16_t _deadzone = 150;
    int16_t mapToRange(uint16_t raw, uint16_t center);
};
