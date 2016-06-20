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

extern uint8_t update_node(const uint16_t macaddr);
void handle_response(   Time_cptr time, MessageResponse_cptr msg,
                        const uint8_t rssi);

void put_response_message(uint8_t n);


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

    uint8_t n = update_node(msg->macaddr);
    if(n == SETTINGS_MAX_NODES)
    {
        WARNING(TIME_FMT "|R|DROP RESP\r\n", TIME_FMT_DATA(*time));
        return;
    }

    request.assignment[n].ttl   = msg->assignment_ttl;
    request.assignment[n].size  = msg->size;
    for(uint8_t s = 0; s < msg->size; ++ s)
        request.assignment[n].slotmask[s] = msg->slotmask[s];

    if(msg->macaddr != device_macaddr)
    {
        if(!msg->ttl)
            return;

        uint8_t size = message_response_get_size(msg);
        MessageResponse *forward = (MessageResponse *) control_txbuffer_get(
                size);

        if(!forward)
        {
            WARNING(TIME_FMT "|R|DROP RESP\r\n", TIME_FMT_DATA(*time));
            return;
        }

        memcpy(forward, msg, size);
        -- forward->ttl;
        control_txbuffer_commit(size);
        return;
    }
}

inline
void put_response_message(uint8_t n)
{
    if(request.assignment[n].ttl > SETTINGS_MAX_HOP)
        return;

    uint8_t size = sizeof(MessageResponse) + request.assignment[n].size;

    MessageResponse *msg = (MessageResponse *) control_txbuffer_get(size);

    if(!msg)
        return;

    msg->kind           = KIND_RESPONSE;
    msg->size           = request.assignment[n].size;
    msg->macaddr        = neighbours.node[n].macaddr;
    msg->ttl            = SETTINGS_MAX_HOP;
    msg->assignment_ttl = request.assignment[n].ttl - SETTINGS_MAX_HOP;
    for(uint8_t s = 0; s < request.assignment[n].size; ++ s)
        msg->slotmask[s] = request.assignment[n].slotmask[s];

    control_txbuffer_commit(size);
    DEBUG(  TIME_FMT "|R|-RESP(-1,0x%04x,%u,%u,%u,0x",
            (uint16_t) 0, (uint32_t) 0, msg->macaddr, msg->ttl,
            msg->assignment_ttl, msg->size);

    for(uint8_t s = 0; s < msg->size; ++ s)
        DEBUG("%02x", msg->slotmask[s]);

    DEBUG(")\r\n");
}

#endif // __AVR__
#endif // __PROTOCOL_MESSAGES_RESPONSE_H__
