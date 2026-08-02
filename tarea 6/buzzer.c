#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "config.h"

void buzzer_init(void)
{
    gpio_set_direction(PIN_BUZZER, GPIO_MODE_OUTPUT);
}

void buzzer_beep(int ms)
{
    gpio_set_level(PIN_BUZZER, 1);
    vTaskDelay(pdMS_TO_TICKS(ms));
    gpio_set_level(PIN_BUZZER, 0);
}