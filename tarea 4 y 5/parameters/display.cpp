#include "display.h"

static const unsigned char PROGMEM logo_bmp[] = {
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x7E,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x7E,0x00,0x00,0x00,0x00,0x00,0x00,
    0x01,0xFF,0x80,0x00,0x00,0x00,0x00,0x00,
    0x03,0xFF,0xC0,0x00,0x00,0x00,0x00,0x00,
    0x03,0xE7,0xC0,0x00,0x00,0x00,0x00,0x00,
    0x07,0xC3,0xE0,0x00,0x00,0x00,0x00,0x00,
    0x0F,0x81,0xF0,0x00,0x00,0x00,0x00,0x00,
    0x0F,0x81,0xF0,0x00,0x00,0x00,0x00,0x00,
    0x1F,0xFF,0xF8,0x00,0x00,0x00,0x00,0x00,
    0x1F,0xFF,0xF8,0x00,0x00,0x00,0x00,0x00,
    0x3F,0x81,0xFC,0x00,0x00,0x00,0x00,0x00,
    0x7F,0x00,0xFE,0x00,0x00,0x00,0x00,0x00,
    0x7E,0x00,0x7E,0x00,0x00,0x00,0x00,0x00,
    0x7C,0x00,0x3E,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};

void Display::begin() {
    _oled = Adafruit_SSD1306(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

    if (!_oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
        Serial.println("[DISPLAY] OLED init failed!");
        return;
    }
    _oled.clearDisplay();
    _oled.setTextColor(SSD1306_WHITE);
    showSplash();
}

void Display::showSplash() {
    _oled.clearDisplay();
    _oled.drawBitmap(0, 0, logo_bmp, 64, 16, SSD1306_WHITE);
    _oled.setTextSize(1);
    _oled.setTextColor(SSD1306_WHITE);
    _oled.setCursor(0, 24);
    _oled.println(F("KAKATA RC433 V1"));
    _oled.setCursor(0, 36);
    _oled.println(F("Controller v1.0"));
    _oled.setCursor(0, 52);
    _oled.println(F("ITLA-HUB 2026"));
    _oled.display();
    delay(2000);
}

void Display::showCalibration() {
    _oled.clearDisplay();
    _oled.setTextSize(1);
    _oled.setTextColor(SSD1306_WHITE);
    _oled.setCursor(10, 20);
    _oled.println(F("CALIBRANDO"));
    _oled.setCursor(10, 34);
    _oled.println(F("Joysticks..."));
    _oled.display();
}

void Display::showError(const char* msg) {
    _oled.clearDisplay();
    _oled.setTextSize(1);
    _oled.setTextColor(SSD1306_WHITE);
    _oled.setCursor(0, 0);
    _oled.println(F("ERROR:"));
    _oled.setCursor(0, 14);
    _oled.println(msg);
    _oled.display();
}

void Display::update(ControllerState& state) {
    unsigned long now = millis();

    if (now - _lastPage > 2000) {
        _page = (_page + 1) % 3;
        _lastPage = now;
    }

    _oled.clearDisplay();

    drawStatus(state);

    switch (_page) {
        case 0: drawMainPage(state); break;
        case 1: drawJoysticks(state); break;
        case 2: drawIMU(state); break;
    }

    _oled.display();
}

void Display::drawStatus(ControllerState& s) {
    _oled.setTextSize(1);
    _oled.setTextColor(SSD1306_WHITE);

    _oled.setCursor(0, 0);
    _oled.print(F("V:"));
    _oled.print(s.battery.voltage, 1);
    _oled.print(F("V "));

    uint8_t bars = s.battery.percentage / 20;
    for (int i = 0; i < 5; i++) {
        _oled.print(i < bars ? F("\x07") : F("\x08"));
    }
    _oled.print(F(" "));

    _oled.print(F("W"));
    _oled.print(s.wifiConnected ? F("+") : F("-"));
    _oled.print(F(" M"));
    _oled.print(s.mqttConnected ? F("+") : F("-"));

    _oled.drawLine(0, 10, 127, 10, SSD1306_WHITE);
}

void Display::drawMainPage(ControllerState& s) {
    _oled.setTextSize(1);
    _oled.setCursor(0, 14);

    _oled.print(F("J0:"));
    _oled.print(s.joy[0].x);
    _oled.print(F(","));
    _oled.print(s.joy[0].y);

    _oled.setCursor(0, 24);
    _oled.print(F("J1:"));
    _oled.print(s.joy[1].x);
    _oled.print(F(","));
    _oled.print(s.joy[1].y);

    _oled.setCursor(0, 36);
    _oled.print(F("G:"));
    _oled.print((int)s.imu.roll);
    _oled.print(F(","));
    _oled.print((int)s.imu.pitch);

    _oled.setCursor(0, 48);
    _oled.print(F("T:"));
    for (int i = 0; i < 4; i++) _oled.print(s.buttons.trigger[i] ? "1" : "0");
    _oled.print(F(" S:"));
    for (int i = 0; i < 4; i++) _oled.print(s.buttons.side[i] ? "1" : "0");

    _oled.setCursor(80, 14);
    _oled.print(F("Hz:"));
    _oled.print((int)s.loopHz);
}

void Display::drawJoysticks(ControllerState& s) {
    drawJoystickGraph(32, 38, s.joy[0].x, s.joy[0].y, "L");
    drawJoystickGraph(96, 38, s.joy[1].x, s.joy[1].y, "R");

    _oled.setTextSize(1);
    _oled.setCursor(20, 58);
    _oled.print(s.joy[0].x);
    _oled.print(F(","));
    _oled.print(s.joy[0].y);

    _oled.setCursor(84, 58);
    _oled.print(s.joy[1].x);
    _oled.print(F(","));
    _oled.print(s.joy[1].y);
}

void Display::drawJoystickGraph(int cx, int cy, int16_t x, int16_t y, const char* label) {
    int r = 14;
    _oled.drawCircle(cx, cy, r, SSD1306_WHITE);
    _oled.drawLine(cx - r, cy, cx + r, cy, SSD1306_WHITE);
    _oled.drawLine(cx, cy - r, cx, cy + r, SSD1306_WHITE);

    int dx = map(x, -100, 100, -r + 2, r - 2);
    int dy = map(y, -100, 100, r - 2, -r + 2);
    _oled.fillCircle(cx + dx, cy + dy, 2, SSD1306_WHITE);

    _oled.setTextSize(1);
    _oled.setCursor(cx - 3, cy - r - 8);
    _oled.print(label);
}

void Display::drawIMU(ControllerState& s) {
    _oled.setTextSize(1);
    _oled.setCursor(0, 14);
    _oled.print(F("ACCEL (m/s2)"));
    _oled.setCursor(0, 24);
    _oled.print(F("X:"));
    _oled.print(s.imu.accelX, 1);
    _oled.print(F(" Y:"));
    _oled.print(s.imu.accelY, 1);

    _oled.setCursor(0, 34);
    _oled.print(F("Z:"));
    _oled.print(s.imu.accelZ, 1);

    _oled.setCursor(0, 46);
    _oled.print(F("GYRO (d/s)"));
    _oled.setCursor(0, 56);
    _oled.print(F("R:"));
    _oled.print((int)s.imu.roll);
    _oled.print(F(" P:"));
    _oled.print((int)s.imu.pitch);
    _oled.print(F(" T:"));
    _oled.print((int)s.imu.yaw);

    _oled.setCursor(100, 24);
    _oled.print(s.imu.temperature, 0);
    _oled.print(F("C"));
}

void Display::drawButtons(ControllerState& s) {
    _oled.setTextSize(1);
    _oled.setCursor(0, 14);
    _oled.print(F("TRIGGERS:"));
    for (int i = 0; i < 4; i++) {
        _oled.setCursor(i * 20 + 10, 24);
        _oled.print(F("T"));
        _oled.print(i);
        _oled.print(F(":"));
        _oled.print(s.buttons.trigger[i] ? "1" : "0");
    }
    _oled.setCursor(0, 38);
    _oled.print(F("SIDE:"));
    for (int i = 0; i < 4; i++) {
        _oled.setCursor(i * 20 + 10, 48);
        _oled.print(F("S"));
        _oled.print(i);
        _oled.print(F(":"));
        _oled.print(s.buttons.side[i] ? "1" : "0");
    }
}
