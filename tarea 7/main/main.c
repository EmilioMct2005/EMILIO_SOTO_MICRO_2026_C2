#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "driver/gpio.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_random.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "mqtt_client.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "config.h"

static const char *TAG = "reaction_game";
static esp_mqtt_client_handle_t s_mqtt_client;
static volatile bool s_mqtt_connected;
static uint32_t s_round;

typedef struct {
    gpio_num_t pin;
    int stable_level;
    int candidate_level;
    int64_t candidate_since_us;
} debounced_button_t;

typedef enum {
    GAME_READY,
    GAME_RANDOM_WAIT,
    GAME_MEASURE,
    GAME_SHOW_RESULT,
    GAME_WAIT_RELEASE
} game_state_t;

static bool button_update(debounced_button_t *button, int64_t now_us,
                          int *new_level, int64_t *edge_time_us)
{
    const int raw_level = gpio_get_level(button->pin);

    if (raw_level != button->candidate_level) {
        button->candidate_level = raw_level;
        button->candidate_since_us = now_us;
        return false;
    }

    if (raw_level != button->stable_level &&
        now_us - button->candidate_since_us >= DEBOUNCE_MS * 1000LL) {
        button->stable_level = raw_level;
        *new_level = raw_level;
        /* Se usa el inicio del cambio estable, no el final del antirrebote. */
        *edge_time_us = button->candidate_since_us;
        return true;
    }

    return false;
}

static void mqtt_send(const char *topic, const char *payload)
{
    if (s_mqtt_client == NULL) {
        ESP_LOGW(TAG, "MQTT no inicializado: %s", payload);
        return;
    }

    if (!s_mqtt_connected) {
        ESP_LOGW(TAG, "MQTT sin conexion; encolando el resultado");
    }

    /* QoS 1 y store=true conservan el mensaje en el outbox si se corta la red. */
    int id = esp_mqtt_client_enqueue(s_mqtt_client, topic, payload, 0, 1, 0, true);
    if (id < 0) {
        ESP_LOGW(TAG, "No se pudo encolar MQTT: %s", payload);
    }
}

static void send_status(const char *status)
{
    char payload[128];
    snprintf(payload, sizeof(payload),
             "{\"ronda\":%lu,\"estado\":\"%s\"}",
             (unsigned long)s_round, status);
    mqtt_send(MQTT_TOPIC_STATUS, payload);
}

static void send_result(int64_t release_us, int64_t reaction_us)
{
    char payload[224];
    snprintf(payload, sizeof(payload),
             "{\"ronda\":%lu,\"liberacion_boton_1_ms\":%.3f,"
             "\"pulsacion_boton_2_ms\":%.3f,\"salida_falsa\":false}",
             (unsigned long)s_round,
             release_us / 1000.0,
             reaction_us / 1000.0);
    mqtt_send(MQTT_TOPIC_RESULTS, payload);
    ESP_LOGI(TAG, "%s", payload);
}

static void send_false_start(const char *reason)
{
    char payload[192];
    snprintf(payload, sizeof(payload),
             "{\"ronda\":%lu,\"salida_falsa\":true,\"motivo\":\"%s\"}",
             (unsigned long)s_round, reason);
    mqtt_send(MQTT_TOPIC_RESULTS, payload);
    ESP_LOGW(TAG, "%s", payload);
}

static void mqtt_event_handler(void *arg, esp_event_base_t base,
                               int32_t event_id, void *event_data)
{
    (void)arg;
    (void)base;
    (void)event_data;

    if (event_id == MQTT_EVENT_CONNECTED) {
        s_mqtt_connected = true;
        ESP_LOGI(TAG, "MQTT conectado");
        esp_mqtt_client_publish(s_mqtt_client, MQTT_TOPIC_STATUS,
                                "{\"estado\":\"dispositivo_conectado\"}",
                                0, 1, 1);
    } else if (event_id == MQTT_EVENT_DISCONNECTED) {
        s_mqtt_connected = false;
        ESP_LOGW(TAG, "MQTT desconectado; se intentara reconectar");
    }
}

static void mqtt_init(void)
{
    const esp_mqtt_client_config_t mqtt_config = {
        .broker.address.uri = MQTT_BROKER_URI,
        .credentials.username = MQTT_USERNAME,
        .credentials.authentication.password = MQTT_PASSWORD,
    };

    s_mqtt_client = esp_mqtt_client_init(&mqtt_config);
    ESP_ERROR_CHECK(s_mqtt_client == NULL ? ESP_FAIL : ESP_OK);
    ESP_ERROR_CHECK(esp_mqtt_client_register_event(
        s_mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL));
    ESP_ERROR_CHECK(esp_mqtt_client_start(s_mqtt_client));
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    (void)arg;
    (void)event_data;

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        s_mqtt_connected = false;
        ESP_LOGW(TAG, "Wi-Fi desconectado; reintentando...");
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ESP_LOGI(TAG, "Wi-Fi conectado");
    }
}

static void wifi_init(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t init_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&init_config));
    ESP_ERROR_CHECK(esp_event_handler_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL));

    wifi_config_t wifi_config = {0};
    strlcpy((char *)wifi_config.sta.ssid, WIFI_SSID,
            sizeof(wifi_config.sta.ssid));
    strlcpy((char *)wifi_config.sta.password, WIFI_PASSWORD,
            sizeof(wifi_config.sta.password));
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

static void gpio_init_game(void)
{
    const gpio_config_t outputs = {
        .pin_bit_mask = (1ULL << PIN_LED_WAIT) | (1ULL << PIN_LED_GO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&outputs));

    const gpio_config_t inputs = {
        .pin_bit_mask = (1ULL << PIN_BUTTON_START) |
                        (1ULL << PIN_BUTTON_REACTION),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&inputs));
    gpio_set_level(PIN_LED_WAIT, 0);
    gpio_set_level(PIN_LED_GO, 0);
}

static void game_task(void *arg)
{
    (void)arg;
    debounced_button_t button_1 = {
        .pin = PIN_BUTTON_START,
        .stable_level = gpio_get_level(PIN_BUTTON_START),
        .candidate_level = gpio_get_level(PIN_BUTTON_START),
        .candidate_since_us = esp_timer_get_time(),
    };
    debounced_button_t button_2 = {
        .pin = PIN_BUTTON_REACTION,
        .stable_level = gpio_get_level(PIN_BUTTON_REACTION),
        .candidate_level = gpio_get_level(PIN_BUTTON_REACTION),
        .candidate_since_us = esp_timer_get_time(),
    };

    game_state_t state = GAME_READY;
    int64_t signal_deadline_us = 0;
    int64_t signal_time_us = 0;
    int64_t release_time_us = -1;
    int64_t reaction_time_us = -1;
    int64_t result_until_us = 0;

    ESP_LOGI(TAG, "Listo: mantenga presionado el boton 1 para iniciar");

    while (true) {
        const int64_t now_us = esp_timer_get_time();
        int level_1 = button_1.stable_level;
        int level_2 = button_2.stable_level;
        int64_t edge_1_us = now_us;
        int64_t edge_2_us = now_us;
        const bool edge_1 = button_update(&button_1, now_us, &level_1, &edge_1_us);
        const bool edge_2 = button_update(&button_2, now_us, &level_2, &edge_2_us);

        switch (state) {
        case GAME_READY:
            if (edge_1 && level_1 == BUTTON_PRESSED_LEVEL &&
                button_2.stable_level != BUTTON_PRESSED_LEVEL) {
                s_round++;
                const uint32_t span = RANDOM_WAIT_MAX_MS - RANDOM_WAIT_MIN_MS + 1;
                const uint32_t wait_ms = RANDOM_WAIT_MIN_MS + (esp_random() % span);
                signal_deadline_us = now_us + wait_ms * 1000LL;
                gpio_set_level(PIN_LED_WAIT, 1);
                send_status("esperando_senal");
                ESP_LOGI(TAG, "Ronda %lu iniciada", (unsigned long)s_round);
                state = GAME_RANDOM_WAIT;
            }
            break;

        case GAME_RANDOM_WAIT:
            if ((edge_1 && level_1 != BUTTON_PRESSED_LEVEL) ||
                (edge_2 && level_2 == BUTTON_PRESSED_LEVEL)) {
                send_false_start(level_1 != BUTTON_PRESSED_LEVEL
                                     ? "boton_1_liberado_antes_del_led"
                                     : "boton_2_pulsado_antes_del_led");
                gpio_set_level(PIN_LED_WAIT, 0);
                gpio_set_level(PIN_LED_GO, 0);
                result_until_us = now_us + RESULT_DISPLAY_MS * 1000LL;
                state = GAME_SHOW_RESULT;
            } else if (now_us >= signal_deadline_us) {
                gpio_set_level(PIN_LED_WAIT, 0);
                gpio_set_level(PIN_LED_GO, 1);
                signal_time_us = esp_timer_get_time();
                release_time_us = -1;
                reaction_time_us = -1;
                send_status("senal_encendida");
                state = GAME_MEASURE;
            }
            break;

        case GAME_MEASURE:
            if (release_time_us < 0 && edge_1 && level_1 != BUTTON_PRESSED_LEVEL) {
                release_time_us = edge_1_us - signal_time_us;
                if (release_time_us < 0) {
                    release_time_us = 0;
                }
            }
            if (reaction_time_us < 0 && edge_2 && level_2 == BUTTON_PRESSED_LEVEL) {
                reaction_time_us = edge_2_us - signal_time_us;
                if (reaction_time_us < 0) {
                    reaction_time_us = 0;
                }
            }
            if (release_time_us >= 0 && reaction_time_us >= 0) {
                send_result(release_time_us, reaction_time_us);
                gpio_set_level(PIN_LED_GO, 0);
                result_until_us = now_us + RESULT_DISPLAY_MS * 1000LL;
                state = GAME_SHOW_RESULT;
            }
            break;

        case GAME_SHOW_RESULT:
            if (now_us >= result_until_us) {
                state = GAME_WAIT_RELEASE;
            }
            break;

        case GAME_WAIT_RELEASE:
            if (button_1.stable_level != BUTTON_PRESSED_LEVEL &&
                button_2.stable_level != BUTTON_PRESSED_LEVEL) {
                send_status("listo");
                ESP_LOGI(TAG, "Listo para otra ronda");
                state = GAME_READY;
            }
            break;
        }

        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void app_main(void)
{
    esp_err_t nvs_result = nvs_flash_init();
    if (nvs_result == ESP_ERR_NVS_NO_FREE_PAGES ||
        nvs_result == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        nvs_result = nvs_flash_init();
    }
    ESP_ERROR_CHECK(nvs_result);

    gpio_init_game();
    wifi_init();
    mqtt_init();
    xTaskCreate(game_task, "game_task", 4096, NULL, 5, NULL);
}
