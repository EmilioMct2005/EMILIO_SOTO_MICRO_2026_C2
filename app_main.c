#ifndef CONFIG_H
#define CONFIG_H

//---------------------- WIFI ----------------------//
#define WIFI_SSID      "TU_WIFI"
#define WIFI_PASSWORD  "TU_PASSWORD"

//---------------------- MQTT ----------------------//
#define MQTT_SERVER    "192.168.1.100"
#define MQTT_PORT      1883

#define TOPIC_CONTROL  "carro/control"
#define TOPIC_STATUS   "carro/status"

//---------------------- OLED ----------------------//
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

//---------------------- JOYSTICKS ----------------------//

#define LX_PIN 34
#define LY_PIN 35

#define RX_PIN 32
#define RY_PIN 33

//---------------------- BOTONES ----------------------//

#define L1_PIN 25
#define L2_PIN 26

#define R1_PIN 27
#define R2_PIN 14

#define A_PIN 4
#define B_PIN 16
#define X_PIN 17
#define Y_PIN 5

#define DEADZONE 5

#endif

#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

#include <ArduinoJson.h>

#include "config.h"

WiFiClient espClient;
PubSubClient mqtt(espClient);

Adafruit_SSD1306 display(
SCREEN_WIDTH,
SCREEN_HEIGHT,
&Wire,
-1);

Adafruit_MPU6050 mpu;
int lx;
int ly;
int rx;
int ry;

int lxCenter=2048;
int lyCenter=2048;

int rxCenter=2048;
int ryCenter=2048;
float roll;
float pitch;

float ax;
float ay;
float az;
bool L1;
bool L2;
bool R1;
bool R2;

bool A;
bool B;
bool X;
bool Y;
void connectWiFi();
void connectMQTT();

void readJoysticks();
void readButtons();

void readMPU();

void updateDisplay();

void sendMQTT();

void calibration();
void setup()
{

Serial.begin(115200);

Wire.begin();

pinMode(L1_PIN,INPUT_PULLUP);
pinMode(L2_PIN,INPUT_PULLUP);
pinMode(R1_PIN,INPUT_PULLUP);
pinMode(R2_PIN,INPUT_PULLUP);

pinMode(A_PIN,INPUT_PULLUP);
pinMode(B_PIN,INPUT_PULLUP);
pinMode(X_PIN,INPUT_PULLUP);
pinMode(Y_PIN,INPUT_PULLUP);

display.begin(
SSD1306_SWITCHCAPVCC,
0x3C);

display.clearDisplay();
display.display();

if(!mpu.begin())
{

while(true);

}

connectWiFi();

mqtt.setServer(
MQTT_SERVER,
MQTT_PORT);

}
void loop()
{

if(!mqtt.connected())
connectMQTT();

mqtt.loop();

readJoysticks();

readButtons();

readMPU();

calibration();

updateDisplay();

sendMQTT();

delay(20);

}
void connectWiFi()
{

WiFi.begin(
WIFI_SSID,
WIFI_PASSWORD);

while(WiFi.status()!=WL_CONNECTED)
{

delay(500);

}

}
void connectMQTT()
{

while(!mqtt.connected())
{

mqtt.connect("ControlRemoto");

delay(500);

}

}
#ifndef JOYSTICK_H
#define JOYSTICK_H

extern int lx;
extern int ly;
extern int rx;
extern int ry;

void readJoysticks();

#endif
#include "joystick.h"
#include "config.h"

// Estas variables están definidas en main.cpp
extern int lx;
extern int ly;
extern int rx;
extern int ry;

extern int lxCenter;
extern int lyCenter;
extern int rxCenter;
extern int ryCenter;

// Filtro exponencial
float flx = 0;
float fly = 0;
float frx = 0;
float fry = 0;

const float alpha = 0.20;

int mapJoystick(int value, int center)
{
    int delta = value - center;

    if (delta > 2047) delta = 2047;
    if (delta < -2048) delta = -2048;

    int salida = (delta * 100) / 2047;

    if (abs(salida) < DEADZONE)
        salida = 0;

    if (salida > 100) salida = 100;
    if (salida < -100) salida = -100;

    return salida;
}

void readJoysticks()
{
    flx = alpha * analogRead(LX_PIN) + (1 - alpha) * flx;
    fly = alpha * analogRead(LY_PIN) + (1 - alpha) * fly;
    frx = alpha * analogRead(RX_PIN) + (1 - alpha) * frx;
    fry = alpha * analogRead(RY_PIN) + (1 - alpha) * fry;

    lx = mapJoystick((int)flx, lxCenter);
    ly = mapJoystick((int)fly, lyCenter);
    rx = mapJoystick((int)frx, rxCenter);
    ry = mapJoystick((int)fry, ryCenter);
}
#ifndef BUTTONS_H
#define BUTTONS_H

void readButtons();

#endif
#include "buttons.h"
#include "config.h"

extern bool L1;
extern bool L2;
extern bool R1;
extern bool R2;

extern bool A;
extern bool B;
extern bool X;
extern bool Y;

void readButtons()
{
    L1 = !digitalRead(L1_PIN);
    L2 = !digitalRead(L2_PIN);

    R1 = !digitalRead(R1_PIN);
    R2 = !digitalRead(R2_PIN);

    A = !digitalRead(A_PIN);
    B = !digitalRead(B_PIN);
    X = !digitalRead(X_PIN);
    Y = !digitalRead(Y_PIN);
}
#include "joystick.h"
#include "buttons.h"
struct ControllerData {
    int lx;
    int ly;
    int rx;
    int ry;

    float roll;
    float pitch;

    float ax;
    float ay;
    float az;

    bool L1;
    bool L2;
    bool R1;
    bool R2;
    bool A;
    bool B;
    bool X;
    bool Y;
};

extern ControllerData control;