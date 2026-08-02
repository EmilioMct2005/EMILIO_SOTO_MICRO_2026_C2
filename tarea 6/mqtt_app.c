#include "mqtt_client.h"
#include "app_fsm.h"

static esp_mqtt_client_handle_t client;

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;

    if(event->event_id == MQTT_EVENT_DATA)
    {
        if(strncmp(event->data, "OPEN", event->data_len) == 0)
            fsm_process(EV_OPEN);
        else if(strncmp(event->data, "CLOSE", event->data_len) == 0)
            fsm_process(EV_CLOSE);
        else if(strncmp(event->data, "STOP", event->data_len) == 0)
            fsm_process(EV_STOP);
    }
}

void mqtt_app_start(void)
{
    esp_mqtt_client_config_t cfg = {
        .broker.address.uri = "mqtt://broker.emqx.io:1883"
    };

    client = esp_mqtt_client_init(&cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
    esp_mqtt_client_subscribe(client, "porton/cmd", 1);
}

void mqtt_publish_state(const char *s)
{
    esp_mqtt_client_publish(client, "porton/estado", s, 0, 1, 0);
}