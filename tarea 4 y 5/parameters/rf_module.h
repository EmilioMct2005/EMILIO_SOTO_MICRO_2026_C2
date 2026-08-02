#pragma once

#include "config.h"
#include "types.h"
#include <RCSwitch.h>

class RFModule {
public:
    void begin();
    void enable();
    void disable();
    void send(RFData& data);
    bool receive(RFData& data);
    bool hasData();

private:
    RCSwitch _rf;
    bool _enabled = false;
    uint8_t calculateChecksum(RFData& data);
};
