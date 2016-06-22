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
//  [5]     unused          - nieużywane
//  [16]    macaddr         - kogo dotyczy przydział
//  [8]     ttl             - dla obsługi multi-hop
//  [8]     assignment_ttl  - jak długo jest ważny
//  [32]    slotmask        - maska przydziału
//

#define KIND_RESPONSE 0x5

typedef struct message_response
{
    uint8_t kind    : 3;
    uint8_t unused  : 5;

    uint16_t    macaddr;
    uint8_t     ttl;
    uint8_t     assignment_ttl;
    uint32_t    slotmask;
} MessageResponse;

typedef const MessageResponse * const MessageResponse_cptr;

inline
uint8_t message_response_get_size(__unused__ MessageResponse_cptr msg)
{
    return sizeof(MessageResponse);
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
    DEBUG(  TIME_FMT "|R|+RESP(%u,0x%04x,%u,%u,0x%08x)\r\n",
            TIME_FMT_DATA(*time), rssi, msg->macaddr, msg->ttl,
            msg->assignment_ttl, msg->slotmask);

    uint8_t n = update_node(msg->macaddr);
    if(n == SETTINGS_MAX_NODES)
    {
        WARNING(TIME_FMT "|R|DROP RESP\r\n", TIME_FMT_DATA(*time));
        return;
    }

    request.assignment[n].ttl       = msg->assignment_ttl;
    request.assignment[n].slotmask  = msg->slotmask;

    if(n)
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

    uint8_t size = sizeof(MessageResponse);

    MessageResponse *msg = (MessageResponse *) control_txbuffer_get(size);

    if(!msg)
        return;

    msg->kind           = KIND_RESPONSE;
    msg->unused         = 0x15; // 0b10101
    msg->macaddr        = neighbours.node[n].macaddr;
    msg->ttl            = SETTINGS_MAX_HOP;
    msg->assignment_ttl = request.assignment[n].ttl - SETTINGS_MAX_HOP;
    msg->slotmask       = request.assignment[n].slotmask;

    control_txbuffer_commit(size);
    DEBUG(  TIME_FMT "|R|-RESP(-1,0x%04x,%u,%u,0x%08x)\r\n",
            (uint16_t) 0, (uint32_t) 0, msg->macaddr, msg->ttl,
            msg->assignment_ttl, msg->slotmask);
}

#endif // __AVR__
#endif // __PROTOCOL_MESSAGES_RESPONSE_H__
