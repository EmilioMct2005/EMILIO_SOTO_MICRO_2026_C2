#pragma once

#include "config.h"
#include "types.h"
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

class Display {
public:
    void begin();
    void update(ControllerState& state);
    void showCalibration();
    void showError(const char* msg);
    void showSplash();

private:
    Adafruit_SSD1306 _oled;
    uint8_t _page = 0;
    unsigned long _lastPage = 0;
    void drawMainPage(ControllerState& s);
    void drawJoysticks(ControllerState& s);
    void drawIMU(ControllerState& s);
    void drawButtons(ControllerState& s);
    void drawBattery(ControllerState& s);
    void drawStatus(ControllerState& s);
    void drawJoystickGraph(int cx, int cy, int16_t x, int16_t y, const char* label);
};
