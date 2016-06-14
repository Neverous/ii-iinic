#ifndef __PROTOCOL_MESSAGES_REQUEST_H__
#define __PROTOCOL_MESSAGES_REQUEST_H__

#include <string.h>

#include "common.h"
#include "synchronization.h"

//
// MessageRequest
//  Żądanie przydziału "łącza". Wiadomość z takim samym seq_id jak poprzednia
//  oraz size=0 oznacza że zwalniam wcześniej otrzymany przydział.
//  rozmiar: 8 bajtów
//
//  [3]     kind                        - typ wiadomości = 0x4
//  [5 + 8] size_high << 8 | size_low   - żądany przydział (liczba 8bajtowych bloków)
//  [16]    macaddr                     - adres MAC nadawcy
//  [16]    dst_macaddr                 - adres MAC odbiorcy
//  [8]     seq_id                      - numer zapytania
//  [8]     ttl                         - dla obsługi multi-hop
//

#define KIND_REQUEST 0x4

typedef struct message_request
{
    uint8_t kind        : 3;
    uint8_t count_high  : 5;

    uint8_t     count_low;
    uint16_t    macaddr;
    uint16_t    dst_macaddr;
    uint8_t     seq_id;
    uint8_t     ttl;
} MessageRequest;

typedef const MessageRequest * const MessageRequest_cptr;

inline
uint8_t message_request_get_size(__unused__ MessageRequest_cptr msg)
{
    return sizeof(MessageRequest);
}

inline
uint16_t message_request_get_count(MessageRequest_cptr msg)
{
    return ((uint16_t) msg->count_high << 8) | msg->count_low;
}


#ifdef __AVR__
////////////////////////////////////////////////////////////////////////////////

struct RequestData
{
    uint8_t     assignment_ttl[64];
    uint8_t     blocks_left;
    uint16_t    dst_macaddr;
    uint8_t     seq_id;
};

extern struct RequestData request;


////////////////////////////////////////////////////////////////////////////////

void handle_request(Time_cptr time, MessageRequest_cptr msg,
                    const uint8_t rssi);


////////////////////////////////////////////////////////////////////////////////

inline
void handle_request(Time_cptr time, MessageRequest_cptr msg,
                    const uint8_t rssi)
{
    DEBUG(  TIME_FMT "|R|+REQ(%u,0x%04x,0x%04x,%u,%u,%u)\r\n",
            TIME_FMT_DATA(*time), rssi, msg->macaddr, msg->dst_macaddr,
            msg->ttl, msg->seq_id, message_request_get_count(msg));

    if(msg->macaddr == device_macaddr)
        return;

#ifndef STATIC_ROOT
    if(synchronization.root.macaddr != device_macaddr)
#else
    if(STATIC_ROOT != device_macaddr)
#endif
    {
        if(!msg->ttl)
            return;

        uint8_t size = message_request_get_size(msg);
        MessageRequest *forward = (MessageRequest *) txbuffer_get(size);

        if(!forward)
        {
            WARNING(TIME_FMT "|R|DROP REQ\r\n", TIME_FMT_DATA(*time));
            return;
        }

        memcpy(forward, msg, size);
        -- forward->ttl;
        txbuffer_commit(size);
        return;
    }

    // TODO
}

#endif // __AVR__
#endif // __PROTOCOL_MESSAGES_REQUEST_H__
