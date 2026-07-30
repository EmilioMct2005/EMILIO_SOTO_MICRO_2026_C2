#include "driver/gpio.h"
#include "config.h"

void leds_init(void)
{
    gpio_set_direction(LED_GREEN, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_YELLOW, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_RED, GPIO_MODE_OUTPUT);
}

void leds_set(bool g, bool y, bool r)
{
    gpio_set_level(LED_GREEN, g);
    gpio_set_level(LED_YELLOW, y);
    gpio_set_level(LED_RED, r);
}