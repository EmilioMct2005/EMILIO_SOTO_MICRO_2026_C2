#include "app_fsm.h"
#include "driver/gpio.h"

extern void motor_open(uint8_t duty);
extern void motor_close(uint8_t duty);
extern void motor_stop(void);
extern void leds_set(bool g, bool y, bool r);
extern void buzzer_beep(int ms);

static fsm_state_t state = ST_INIT;

fsm_state_t fsm_get_state(void)
{
    return state;
}

void fsm_init(void)
{
    state = ST_CERRADO;
    leds_set(0,0,1);
}

void fsm_process(fsm_event_t ev)
{
    switch(state)
    {
        case ST_CERRADO:
            if(ev == EV_OPEN)
            {
                state = ST_ABRIENDO;
                motor_open(70);
                leds_set(0,1,0);
                buzzer_beep(80);
            }
            break;

        case ST_ABRIENDO:
            if(ev == EV_LIMIT_OPEN)
            {
                state = ST_ABIERTO;
                motor_stop();
                leds_set(1,0,0);
            }
            if(ev == EV_STOP)
            {
                state = ST_STOP;
                motor_stop();
            }
            break;

        case ST_ABIERTO:
            if(ev == EV_CLOSE)
            {
                state = ST_CERRANDO;
                motor_close(70);
                leds_set(0,1,0);
                buzzer_beep(80);
            }
            break;

        case ST_CERRANDO:
            if(ev == EV_LIMIT_CLOSE)
            {
                state = ST_CERRADO;
                motor_stop();
                leds_set(0,0,1);
            }
            else if(ev == EV_PHOTO)
            {
                state = ST_OBSTACULO;
                motor_stop();
                buzzer_beep(200);
                motor_open(70);
            }
            else if(ev == EV_STOP)
            {
                state = ST_STOP;
                motor_stop();
            }
            break;

        case ST_OBSTACULO:
            if(ev == EV_LIMIT_OPEN)
            {
                state = ST_ABIERTO;
                motor_stop();
                leds_set(1,0,0);
            }
            break;

        case ST_STOP:
            if(ev == EV_OPEN)
            {
                state = ST_ABRIENDO;
                motor_open(70);
            }
            else if(ev == EV_CLOSE)
            {
                state = ST_CERRANDO;
                motor_close(70);
            }
            break;

        default:
            break;
    }
}