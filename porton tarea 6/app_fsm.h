#pragma once

typedef enum {
    ST_INIT,
    ST_CERRADO,
    ST_ABRIENDO,
    ST_ABIERTO,
    ST_CERRANDO,
    ST_STOP,
    ST_OBSTACULO,
    ST_ERROR
} fsm_state_t;

typedef enum {
    EV_NONE,
    EV_OPEN,
    EV_CLOSE,
    EV_STOP,
    EV_LIMIT_OPEN,
    EV_LIMIT_CLOSE,
    EV_PHOTO
} fsm_event_t;

void fsm_init(void);
void fsm_process(fsm_event_t ev);
fsm_state_t fsm_get_state(void);