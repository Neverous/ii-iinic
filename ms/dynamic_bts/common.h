#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdint.h>

#include "config.h"
#include "iinic_wrapper.h"
#include "usart_complex.h"

typedef const uint8_t * const uint8_t_cptr;

extern uint16_t device_macaddr;
extern uint8_t rxbuffer[SETTINGS_RXBUFFER_SIZE];
extern uint8_t txbuffer[SETTINGS_TXBUFFER_SIZE];
extern uint8_t *txbuffer_ptr;


uint8_t *txbuffer_get(uint8_t bytes)
{
    if(txbuffer_ptr + bytes + 2 > txbuffer + SETTINGS_TXBUFFER_SIZE)
        return NULL;

    return txbuffer_ptr;
}

void txbuffer_commit(uint8_t bytes)
{
    uint8_t *after = txbuffer_ptr + bytes;
    *(uint16_t *) after = crc16(txbuffer_ptr, bytes);
    txbuffer_ptr = after + 2;
}

#endif // __COMMON_H__
