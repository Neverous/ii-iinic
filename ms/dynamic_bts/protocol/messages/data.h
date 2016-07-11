#ifndef __PROTOCOL_MESSAGES_DATA_H__
#define __PROTOCOL_MESSAGES_DATA_H__

#include "common.h"
#include "neighbours.h"

//
// MessageData
//  Wiadomość z danymi (pakiet),
//  rozmiar: 5-253 bajty
//
//  [3]     kind        - typ wiadomości = 0x6
//  [5]     size        - rozmiar = 0-31 (*8 = 0 - 248 bajtów)
//                        liczony w 8 bajtowych blokach
//  [16]    macaddr     - adres MAC nadawcy
//  [16]    dst_macaddr - adres MAC odbiorcy
//  [*]     data[]      - dane
//

#define KIND_DATA 0x6

typedef struct message_data
{
    uint8_t kind    : 3;
    uint8_t size    : 5;

    uint16_t    macaddr;
    uint16_t    dst_macaddr;
    uint8_t     data[];
} MessageData;

typedef const MessageData * const MessageData_cptr;

inline
uint8_t message_data_get_size(MessageData_cptr msg)
{
    return sizeof(MessageData) + msg->size * 8;
}

#ifdef __AVR__
////////////////////////////////////////////////////////////////////////////////

struct DataData
{
    struct
    {
        uint16_t in[SETTINGS_MAX_NODES];
        uint16_t out[SETTINGS_MAX_NODES];
    } stats;
};

extern struct DataData data;

extern uint8_t update_node(const uint16_t macaddr);
void handle_data(Time_cptr time, MessageData_cptr msg, const uint8_t rssi);
void put_data_message(uint8_t destination, uint8_t blocks);


////////////////////////////////////////////////////////////////////////////////

void handle_data(   Time_cptr time,
                    MessageData_cptr msg,
                    const uint8_t rssi)
{
    DEBUG(  TIME_FMT "|R|+DATA(%u,0x%04x,0x%04x,%u)\r\n",
            TIME_FMT_DATA(*time), rssi, msg->macaddr, msg->dst_macaddr,
            msg->size);

    if(msg->dst_macaddr != device_macaddr)
        return;

    uint8_t n = update_node(msg->macaddr);
    if(n == SETTINGS_MAX_NODES)
        return;

    ++ data.stats.in[n];
}

void put_data_message(uint8_t destination, uint8_t blocks)
{
    uint8_t size = sizeof(MessageData) + blocks * 8;
    MessageData *msg = (MessageData *) data_txbuffer_get(size);
    if(!msg)
        return;

    msg->kind           = KIND_DATA;
    msg->size           = blocks;
    msg->macaddr        = device_macaddr;
    msg->dst_macaddr    = neighbours.node[destination].macaddr;
    for(uint8_t d = 0; d < blocks * 8; ++ d)
        msg->data[d] = 0x55;

    data_txbuffer_commit(size);
    ++ data.stats.out[destination];
    DEBUG(  TIME_NULL "|R|-DATA(0x%04x,0x%04x,%u)\r\n",
            msg->macaddr, msg->dst_macaddr, msg->size);
}

#endif // __AVR__
#endif // __PROTOCOL_MESSAGES_DATA_H__
