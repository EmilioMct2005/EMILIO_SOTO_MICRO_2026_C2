#include "driver/ledc.h"
#include "driver/gpio.h"
#include "config.h"

void motor_init(void)
{
    ledc_timer_config_t timer = {
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .freq_hz = 20000,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer);

    ledc_channel_config_t ch = {
        .gpio_num = PIN_PWM,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0
    };
    ledc_channel_config(&ch);

    gpio_set_direction(PIN_DIR, GPIO_MODE_OUTPUT);
}

static void motor_set(uint8_t duty)
{
    uint32_t d = duty * 1023 / 100;
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, d);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
}

void motor_open(uint8_t duty)
{
    gpio_set_level(PIN_DIR, 1);
    motor_set(duty);
}

void motor_close(uint8_t duty)
{
    gpio_set_level(PIN_DIR, 0);
    motor_set(duty);
}

void motor_stop(void)
{
    motor_set(0);
}