#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdint.h>

#include "config.h"
#include "iinic_wrapper.h"
#include "usart_complex.h"

typedef const uint8_t * const uint8_t_cptr;

extern uint16_t device_macaddr;
extern uint8_t *control_txbuffer_ptr;
extern uint8_t *data_txbuffer;
extern uint8_t *data_txbuffer_ptr;
extern uint8_t control_txbuffer[SETTINGS_CONTROL_TXBUFFER_SIZE];
extern uint8_t rxbuffer[SETTINGS_RXBUFFER_SIZE];


uint8_t *control_txbuffer_get(uint8_t bytes)
{
    if(control_txbuffer_ptr + bytes + 2 >
            control_txbuffer + SETTINGS_CONTROL_TXBUFFER_SIZE)
        return NULL;

    return control_txbuffer_ptr;
}

uint8_t *data_txbuffer_get(uint8_t bytes)
{
    if(data_txbuffer_ptr + bytes + 2 >
            data_txbuffer + SETTINGS_DATA_TXBUFFER_SIZE)
        return NULL;

    return data_txbuffer_ptr;
}

void control_txbuffer_commit(uint8_t bytes)
{
    uint8_t *after = control_txbuffer_ptr + bytes;
    *(uint16_t *) after = crc16(control_txbuffer_ptr, bytes);
    control_txbuffer_ptr = after + 2;
}

void data_txbuffer_commit(uint8_t bytes)
{
    uint8_t *after = data_txbuffer_ptr + bytes;
    *(uint16_t *) after = crc16(data_txbuffer_ptr, bytes);
    data_txbuffer_ptr = after + 2;
}

#endif // __COMMON_H__
