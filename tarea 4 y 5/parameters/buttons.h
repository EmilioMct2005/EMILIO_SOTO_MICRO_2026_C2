#pragma once

#include "config.h"
#include "types.h"

class Buttons {
public:
    void begin();
    void update();
    ButtonEvent getEvent();
    ButtonState& getState();

private:
    ButtonState _state;
    ButtonState _lastState;
    uint32_t _holdStart[8] = {};
    uint8_t _pinMap[8];
    bool _pinInverted[8];
    bool _eventPending = false;
    ButtonEvent _event;
};

class LEDs {
public:
    void begin();
    void update(ControllerState& state);
    void setAll(bool on);
    void blink(uint8_t led, int times);
    void setPattern(uint8_t pattern);

private:
    uint8_t _pins[6] = {LED_1_PIN, LED_2_PIN, LED_3_PIN, LED_4_PIN, LED_5_PIN, LED_6_PIN};
    unsigned long _lastBlink = 0;
    bool _blinkState = false;
};
