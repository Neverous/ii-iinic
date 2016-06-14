#ifndef __PROTOCOL_MESSAGES_RESPONSE_H__
#define __PROTOCOL_MESSAGES_RESPONSE_H__

#include <string.h>

#include "common.h"
#include "request.h"

//
// MessageResponse
//  Odpowiedź na żądanie przydziału.
//  W postaci maski bitowej przydziałów oraz liczby rund przez które jest ważny.
//  rozmiar: 5 bajtów
//
//  [3]     kind            - typ wiadomości = 0x7
//  [5]     size            - rozmiar przydziału
//  [16]    macaddr         - kogo dotyczy przydział
//  [8]     ttl             - dla obsługi multi-hop
//  [8]     assignment_ttl  - jak długo jest ważny
//  [*]     slotmask[]      - maska przydziału
//

#define KIND_RESPONSE 0x5

typedef struct message_response
{
    uint8_t kind    : 3;
    uint8_t size    : 5;

    uint16_t    macaddr;
    uint8_t     ttl;
    uint8_t     assignment_ttl;
    uint8_t     slotmask[];
} MessageResponse;

typedef const MessageResponse * const MessageResponse_cptr;

inline
uint8_t message_response_get_size(MessageResponse_cptr msg)
{
    return sizeof(MessageResponse) + msg->size;
}


#ifdef __AVR__
////////////////////////////////////////////////////////////////////////////////

void handle_response(   Time_cptr time, MessageResponse_cptr msg,
                        const uint8_t rssi);


////////////////////////////////////////////////////////////////////////////////

inline
void handle_response(   Time_cptr time, MessageResponse_cptr msg,
                        const uint8_t rssi)
{
    DEBUG(  TIME_FMT "|R|+RESP(%u,0x%04x,%u,%u,%u,0x",
            TIME_FMT_DATA(*time), rssi, msg->macaddr, msg->ttl,
            msg->assignment_ttl, msg->size);

    for(uint8_t s = 0; s < msg->size; ++ s)
        DEBUG("%02x", msg->slotmask[s]);

    DEBUG(")\r\n");

    if(msg->macaddr != device_macaddr)
    {
        if(!msg->ttl)
            return;

        uint8_t size = message_response_get_size(msg);
        MessageResponse *forward = (MessageResponse *) txbuffer_get(size);

        if(!forward)
        {
            WARNING(TIME_FMT "|R|DROP RESP\r\n", TIME_FMT_DATA(*time));
            return;
        }

        memcpy(forward, msg, size);
        -- forward->ttl;
        txbuffer_commit(size);
        return;
    }

    // TODO: check if not already got response
    uint8_t step = 8 / msg->size;
    for(uint8_t m = 0; m < msg->size; ++ m)
    {
        uint8_t start = m * 8 * step;
        for(uint8_t b = 0; b < 8; ++ b) if(msg->slotmask[m] & (1 << b))
            for(uint8_t s = start + b * step; s < start + (b + 1) * step; ++ s)
                request.assignment_ttl[s] = max(
                    request.assignment_ttl[s], msg->assignment_ttl);
    }
}

#endif // __AVR__
#endif // __PROTOCOL_MESSAGES_RESPONSE_H__
