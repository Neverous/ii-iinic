#ifndef __PROTOCOL_MESSAGES_GATHER_H__
#define __PROTOCOL_MESSAGES_GATHER_H__

#include "common.h"
#include "data.h"

//
// MessageGather
//  Statystyki pakietów (wiadomości danych)
//  rozmiar: 1 + SETTINGS_MAX_NODES * 6 bajtów
//
//  [3]                         kind    - typ wiadomości = 0x7
//  [5]                         ttl     - dla obsługi multi-hop
//  [16 * SETTINGS_MAX_NODES]   macaddr - adresy MAC odpowiadające statystykom
//  [16 * SETTINGS_MAX_NODES]   in      - statystyka przychodzących wiadomości
//  [16 * SETTINGS_MAX_NODES]   out     - statystyka wychodzących wiadomości
//

#define KIND_GATHER 0x7

typedef struct message_gather
{
    uint8_t kind    : 3;
    uint8_t ttl     : 5;

    uint16_t    macaddr[SETTINGS_MAX_NODES];
    uint16_t    in[SETTINGS_MAX_NODES];
    uint16_t    out[SETTINGS_MAX_NODES];
} MessageGather;

typedef const MessageGather * const MessageGather_cptr;

inline
uint8_t message_gather_get_size(__unused__ MessageGather_cptr msg)
{
    return sizeof(MessageGather);
}

#ifdef __AVR__
////////////////////////////////////////////////////////////////////////////////

void handle_gather(Time_cptr time, MessageGather_cptr msg, const uint8_t rssi);
void put_gather_message(void);


////////////////////////////////////////////////////////////////////////////////

void handle_gather( Time_cptr time,
                    MessageGather_cptr msg,
                    const uint8_t rssi)
{
    DEBUG(  TIME_FMT "|R|+GAT(%u,%u, 0x%04x)\r\n",
            TIME_FMT_DATA(*time), rssi, msg->ttl, msg->macaddr[0]);

    if(msg->macaddr[0] == device_macaddr)
        return;

    uint8_t size = message_gather_get_size(msg);
#ifdef __USART_COMPLEX__
    if(ping.mode & P_MODE_MONITOR)
    {
        usart_push_block((uint8_t *) msg, size + 2, true); // +2 dla CRC
        DEBUG(  TIME_FMT "|U|-GAT(%u, 0x%04x)\r\n",
                TIME_FMT_DATA(*time), msg->ttl, msg->macaddr[0]);
    }
#endif // __USART_COMPLEX__

#ifndef STATIC_ROOT
    if(synchronization.root.macaddr != device_macaddr)
#else
    if(STATIC_ROOT != device_macaddr)
#endif
    {
        if(!msg->ttl)
            return;

        MessageGather *forward = (MessageGather *) control_txbuffer_get(size);
        if(!forward)
        {
            WARNING(TIME_FMT "|R|DROP GAT\r\n", TIME_FMT_DATA(*time));
            return;
        }

        memcpy(forward, msg, size);
        -- forward->ttl;
        control_txbuffer_commit(size);
        return;
    }
}

void put_gather_message(void)
{
    uint8_t size = sizeof(MessageGather);
    MessageGather *msg = (MessageGather *) control_txbuffer_get(size);
    if(!msg)
        return;

    msg->kind   = KIND_GATHER;
    msg->ttl    = SETTINGS_MAX_HOP;
    for(uint8_t n = 0; n < SETTINGS_MAX_NODES; ++ n)
        msg->macaddr[n] = neighbours.node[n].macaddr;

    memcpy(msg->in, data.stats.in, SETTINGS_MAX_NODES * sizeof(uint16_t));
    memcpy(msg->out, data.stats.out, SETTINGS_MAX_NODES * sizeof(uint16_t));

    control_txbuffer_commit(size);
#ifdef __USART_COMPLEX__
    if(ping.mode & P_MODE_MONITOR)
    {
        usart_push_block((uint8_t *) msg, size + 2, true); // +2 dla CRC
        DEBUG(  TIME_NULL "|U|-GAT(%u,0x%04x)\r\n",
                msg->ttl, msg->macaddr[0]);
    }
#endif // __USART_COMPLEX__

    DEBUG(  TIME_NULL "|R|-GAT(%u,0x%04x)\r\n",
            msg->ttl, msg->macaddr[0]);
}

#endif // __AVR__
#endif // __PROTOCOL_MESSAGES_GATHER_H__
