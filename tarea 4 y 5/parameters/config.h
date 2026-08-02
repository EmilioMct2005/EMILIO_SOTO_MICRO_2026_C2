#pragma once

#include <Arduino.h>

// ============ PIN MAP - KAKATA RC433 V1 ============

// Joystick 0 (Left)
#define JOY0_X_PIN      5   // ADC_JOY0_MD
#define JOY0_Y_PIN      4   // ADC_JOY0_MT
#define JOY0_BTN_PIN    3   // JOY0_BTN

// Joystick 1 (Right)
#define JOY1_X_PIN      1   // ADC_JOY1_MD
#define JOY1_Y_PIN      2   // ADC_JOY1_MT
#define JOY1_BTN_PIN    46  // JOY1_BTN

// Trigger buttons (front face - 4 gatillos)
#define BTN_TRIGGER_0   9   // BTN_0
#define BTN_TRIGGER_1   11  // BTN_1
#define BTN_TRIGGER_2   10  // BTN_2
#define BTN_TRIGGER_3   12  // BTN_3

// Side buttons (4 botones laterales)
#define BTN_SIDE_1      42  // BTNL1
#define BTN_SIDE_2      41  // BTNL2
#define BTN_SIDE_3      40  // BTNL3
#define BTN_SIDE_4      39  // BTNL4

// I2C
#define I2C_SDA_PIN     6   // I2C_SDA
#define I2C_SCL_PIN     7   // I2C_SCL

// OLED Display
#define OLED_ADDR       0x3C
#define OLED_WIDTH      128
#define OLED_HEIGHT     64

// MPU6050
#define MPU6050_ADDR    0x68
#define MPU_INT_PIN     16  // INT_MPU

// RF 433MHz
#define RF_TX_PIN       17  // DATA_RF_SEND
#define RF_RX_PIN       18  // DATA_RF_RECEIVED
#define RF_ENABLE_PIN   15  // RF_ENABLE

// LEDs
#define LED_1_PIN       45  // LED1
#define LED_2_PIN       48  // LED2
#define LED_3_PIN       47  // LED3
#define LED_4_PIN       21  // LED4
#define LED_5_PIN       14  // LED5
#define LED_6_PIN       13  // LED6

// Battery
#define VBAT_PIN        8   // VBAT
#define VBAT_DIVIDER_RATIO  4.07f  // (100K+22K)/22K
#define VBAT_FULL       4.2f
#define VBAT_EMPTY      3.3f

// ADC
#define ADC_RESOLUTION  12
#define ADC_MAX         4095
#define ADC_CENTER      2048

// ============ NETWORK ============
#define WIFI_SSID       "KAKATA-RC"
#define WIFI_PASS       "kakata1234"
#define MQTT_BROKER     "broker.emqx.io"
#define MQTT_PORT       1883
#define MQTT_TOPIC_BASE "kakata/rc433"
#define MQTT_CLIENT_ID  "kakata_rc433_01"

// ============ TIMING ============
#define LOOP_RATE_HZ    50
#define DISPLAY_RATE_MS 100
#define MQTT_RATE_MS    200
#define RF_RATE_MS      50
#define BATTERY_RATE_MS 2000
#define LED_BLINK_MS    500

// ============ CALIBRATION ============
#define CALIB_BUTTON_1  BTN_SIDE_1
#define CALIB_BUTTON_2  BTN_SIDE_2
#define CALIB_HOLD_MS   3000

// ============ RF PROTOCOL ============
#define RF_PROTOCOL     1
#define RF_BIT_LENGTH   24
