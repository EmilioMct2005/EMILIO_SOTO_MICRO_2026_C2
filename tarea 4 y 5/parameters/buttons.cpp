#include "buttons.h"

void Buttons::begin() {
    _pinMap[0] = BTN_TRIGGER_0;  _pinInverted[0] = true;
    _pinMap[1] = BTN_TRIGGER_1;  _pinInverted[1] = true;
    _pinMap[2] = BTN_TRIGGER_2;  _pinInverted[2] = true;
    _pinMap[3] = BTN_TRIGGER_3;  _pinInverted[3] = true;
    _pinMap[4] = BTN_SIDE_1;     _pinInverted[4] = true;
    _pinMap[5] = BTN_SIDE_2;     _pinInverted[5] = true;
    _pinMap[6] = BTN_SIDE_3;     _pinInverted[6] = true;
    _pinMap[7] = BTN_SIDE_4;     _pinInverted[7] = true;

    for (int i = 0; i < 8; i++) {
        pinMode(_pinMap[i], INPUT_PULLUP);
    }
    _state = {};
    _lastState = {};
}

void Buttons::update() {
    _lastState = _state;
    _eventPending = false;

    for (int i = 0; i < 4; i++) {
        bool raw = digitalRead(_pinMap[i]);
        _state.trigger[i] = _pinInverted[i] ? !raw : raw;
    }
    for (int i = 0; i < 4; i++) {
        bool raw = digitalRead(_pinMap[i + 4]);
        _state.side[i] = _pinInverted[i + 4] ? !raw : raw;
    }

    for (int i = 0; i < 8; i++) {
        bool current = (i < 4) ? _state.trigger[i] : _state.side[i - 4];
        bool prev = (i < 4) ? _lastState.trigger[i] : _lastState.side[i - 4];

        if (current && !prev) {
            _holdStart[i] = millis();
        } else if (!current && prev) {
            _eventPending = true;
            _event.type = ButtonEvent::RELEASE;
            _event.index = i;
            _event.isTrigger = (i < 4);
        } else if (current && prev && _holdStart[i] > 0) {
            if (millis() - _holdStart[i] > CALIB_HOLD_MS) {
                _eventPending = true;
                _event.type = ButtonEvent::HOLD;
                _event.index = i;
                _event.isTrigger = (i < 4);
                _holdStart[i] = 0;
            }
        }
    }
}

ButtonEvent Buttons::getEvent() {
    if (_eventPending) {
        _eventPending = false;
        return _event;
    }
    return {ButtonEvent::NONE, 0, false};
}

ButtonState& Buttons::getState() {
    return _state;
}

// ===== LEDs =====

void LEDs::begin() {
    for (int i = 0; i < 6; i++) {
        pinMode(_pins[i], OUTPUT);
        digitalWrite(_pins[i], LOW);
    }
}

void LEDs::update(ControllerState& state) {
    digitalWrite(_pins[0], state.wifiConnected ? HIGH : LOW);
    digitalWrite(_pins[1], state.mqttConnected ? HIGH : LOW);
    digitalWrite(_pins[2], state.rfEnabled ? HIGH : LOW);
    digitalWrite(_pins[3], state.battery.percentage > 20 ? HIGH : LOW);

    unsigned long now = millis();
    if (now - _lastBlink > LED_BLINK_MS) {
        _blinkState = !_blinkState;
        _lastBlink = now;
    }
    digitalWrite(_pins[4], _blinkState ? HIGH : LOW);
    digitalWrite(_pins[5], state.joy[0].pressed || state.joy[1].pressed ? HIGH : LOW);
}

void LEDs::setAll(bool on) {
    for (int i = 0; i < 6; i++) {
        digitalWrite(_pins[i], on ? HIGH : LOW);
    }
}

void LEDs::blink(uint8_t led, int times) {
    for (int i = 0; i < times; i++) {
        digitalWrite(_pins[led], HIGH);
        delay(150);
        digitalWrite(_pins[led], LOW);
        delay(150);
    }
}

void LEDs::setPattern(uint8_t pattern) {
    for (int i = 0; i < 6; i++) {
        digitalWrite(_pins[i], (pattern >> i) & 1);
    }
}
