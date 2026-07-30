#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "config.h"
#include "app_fsm.h"

extern void motor_init(void);
extern void leds_init(void);
extern void buzzer_init(void);
extern void mqtt_app_start(void);

static void inputs_init(void)
{
    gpio_set_direction(PIN_BTN_OPEN, GPIO_MODE_INPUT);
    gpio_set_direction(PIN_BTN_CLOSE, GPIO_MODE_INPUT);
    gpio_set_direction(PIN_BTN_STOP, GPIO_MODE_INPUT);
    gpio_set_direction(PIN_LIMIT_OPEN, GPIO_MODE_INPUT);
    gpio_set_direction(PIN_LIMIT_CLOSE, GPIO_MODE_INPUT);
    gpio_set_direction(PIN_PHOTO, GPIO_MODE_INPUT);
}

static void timer_callback(void* arg)
{
    if(gpio_get_level(PIN_BTN_OPEN))
        fsm_process(EV_OPEN);

    if(gpio_get_level(PIN_BTN_CLOSE))
        fsm_process(EV_CLOSE);

    if(gpio_get_level(PIN_BTN_STOP))
        fsm_process(EV_STOP);

    if(gpio_get_level(PIN_LIMIT_OPEN))
        fsm_process(EV_LIMIT_OPEN);

    if(gpio_get_level(PIN_LIMIT_CLOSE))
        fsm_process(EV_LIMIT_CLOSE);

    if(gpio_get_level(PIN_PHOTO))
        fsm_process(EV_PHOTO);
}

void app_main(void)
{
    motor_init();
    leds_init();
    buzzer_init();
    inputs_init();

    fsm_init();

    const esp_timer_create_args_t t = {
        .callback = timer_callback,
        .name = "scan50ms"
    };

    esp_timer_handle_t timer;
    esp_timer_create(&t, &timer);
    esp_timer_start_periodic(timer, 50000); // 50ms

    mqtt_app_start();

    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}