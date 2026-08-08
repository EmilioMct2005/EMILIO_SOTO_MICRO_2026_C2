#pragma once

/* Red Wi-Fi de 2.4 GHz. */
#define WIFI_SSID               "TU_WIFI"
#define WIFI_PASSWORD           "TU_CLAVE"

/* Ejemplos: mqtt://192.168.1.50:1883 o mqtts://servidor:8883 */
#define MQTT_BROKER_URI         "mqtt://192.168.1.50:1883"
#define MQTT_USERNAME           ""
#define MQTT_PASSWORD           ""
#define MQTT_TOPIC_RESULTS      "juego/reaccion/resultados"
#define MQTT_TOPIC_STATUS       "juego/reaccion/estado"

/* Pines sugeridos para un ESP32 DevKit clásico. */
#define PIN_BUTTON_START        18
#define PIN_BUTTON_REACTION     19
#define PIN_LED_WAIT            25
#define PIN_LED_GO              26

/* Los botones conectan el GPIO a GND y usan pull-up interno. */
#define BUTTON_PRESSED_LEVEL    0

#define RANDOM_WAIT_MIN_MS      2000
#define RANDOM_WAIT_MAX_MS      5000
#define DEBOUNCE_MS             15
#define RESULT_DISPLAY_MS       1500
