#include "rf_module.h"

void RFModule::begin() {
    pinMode(RF_ENABLE_PIN, OUTPUT);
    digitalWrite(RF_ENABLE_PIN, LOW);

    _rf.enableTransmit(RF_TX_PIN);
    _rf.enableReceive(RF_RX_PIN);
    _rf.setProtocol(RF_PROTOCOL);
    _rf.setBitLength(RF_BIT_LENGTH);
    _rf.setRepeatTransmit(3);

    Serial.println("[RF] Initialized");
}

void RFModule::enable() {
    digitalWrite(RF_ENABLE_PIN, HIGH);
    _enabled = true;
    Serial.println("[RF] Enabled");
}

void RFModule::disable() {
    digitalWrite(RF_ENABLE_PIN, LOW);
    _enabled = false;
    Serial.println("[RF] Disabled");
}

uint8_t RFModule::calculateChecksum(RFData& data) {
    uint8_t sum = data.joy0X ^ data.joy0Y ^ data.joy1X ^ data.joy1Y
                ^ data.buttons ^ data.gyroX ^ data.gyroY;
    return sum ^ 0xAA;
}

void RFModule::send(RFData& data) {
    if (!_enabled) return;

    data.checksum = calculateChecksum(data);

    uint32_t packet = 0;
    packet |= ((uint32_t)data.joy0X & 0xFF) << 16;
    packet |= ((uint32_t)data.joy0Y & 0xFF) << 8;
    packet |= ((uint32_t)data.buttons & 0xFF);

    _rf.send(packet, RF_BIT_LENGTH);

    packet = 0;
    packet |= ((uint32_t)data.joy1X & 0xFF) << 16;
    packet |= ((uint32_t)data.joy1Y & 0xFF) << 8;
    packet |= ((uint32_t)data.gyroX & 0xFF);

    _rf.send(packet, RF_BIT_LENGTH);

    packet = 0;
    packet |= ((uint32_t)data.gyroY & 0xFF) << 16;
    packet |= ((uint32_t)data.checksum & 0xFF) << 8;

    _rf.send(packet, RF_BIT_LENGTH);
}

bool RFModule::hasData() {
    return _rf.available();
}

bool RFModule::receive(RFData& data) {
    if (!_rf.available()) return false;

    uint32_t val = _rf.getReceivedValue();
    _rf.resetAvailable();

    if (val == 0) return false;

    data.buttons = val & 0xFF;
    data.joy0X = (val >> 16) & 0xFF;
    data.joy0Y = (val >> 8) & 0xFF;

    return true;
}
