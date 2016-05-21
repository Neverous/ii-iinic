#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdint.h>

#include "config.h"

#define __unused__ __attribute__((unused))


enum EventTypes
{
    MAIN_EVENT              = 0,
    INITIALIZATION_EVENT    = 1,
    TDMA_EVENT              = 2,
    EVENT_MASK              = 3,
};

typedef const uint8_t * const uint8_t_cptr;


extern uint8_t rxbuffer[SETTINGS_RXBUFFER_SIZE];
extern uint8_t txbuffer[SETTINGS_TXBUFFER_SIZE];
extern uint8_t *txbuffer_ptr;

extern uint8_t sensor_id;


#include "iinic_wrapper.h"

#endif // __COMMON_H__
