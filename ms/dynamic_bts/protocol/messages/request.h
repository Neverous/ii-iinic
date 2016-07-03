#ifndef __PROTOCOL_MESSAGES_REQUEST_H__
#define __PROTOCOL_MESSAGES_REQUEST_H__

#include <string.h>

#include "common.h"
#include "synchronization.h"

//
// MessageRequest
//  Żądanie przydziału "łącza". Wiadomość z takim samym seq_id jak poprzednia
//  oraz size=0 oznacza że zwalniam wcześniej otrzymany przydział.
//  rozmiar: 7 bajtów
//
//  [3]     kind                        - typ wiadomości = 0x4
//  [5 + 8] size_high << 8 | size_low   - żądany przydział (liczba 8bajtowych bloków)
//  [16]    macaddr                     - adres MAC nadawcy
//  [16]    dst_macaddr                 - adres MAC odbiorcy
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

inline
void message_request_set_count(MessageRequest *msg, uint16_t count)
{
    msg->count_low = count;
    count <<= 8;
    msg->count_high = count;
}


#ifdef __AVR__
////////////////////////////////////////////////////////////////////////////////

typedef struct assignment
{
    uint8_t     ttl;
    uint8_t     priority;
    uint32_t    slotmask;
} Assignment;

typedef struct request
{
    uint8_t source;
    uint8_t destination;
    uint16_t size;
} Request;

struct RequestData
{
    uint16_t    blocks_left;
    uint8_t     destination;
    Assignment  assignment[SETTINGS_MAX_NODES];
    uint8_t     queue_counter;
    uint8_t     queue_size;
    Request     queue[SETTINGS_REQUEST_QUEUE_SIZE];
};

extern struct RequestData request;


////////////////////////////////////////////////////////////////////////////////

extern void put_response_message(uint8_t n);
void handle_request(Time_cptr time, MessageRequest_cptr msg,
                    const uint8_t rssi);

void put_request_message(void);
void queue_request(uint8_t source, uint8_t destination, uint16_t size);

void validate_request(void);


////////////////////////////////////////////////////////////////////////////////

inline
void handle_request(Time_cptr time, MessageRequest_cptr msg,
                    const uint8_t rssi)
{
    DEBUG(  TIME_FMT "|R|+REQ(%u,0x%04x,0x%04x,%u,%u)\r\n",
            TIME_FMT_DATA(*time), rssi, msg->macaddr, msg->dst_macaddr,
            msg->ttl, message_request_get_count(msg));

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
        MessageRequest *forward = (MessageRequest *) control_txbuffer_get(size);

        if(!forward)
        {
            WARNING(TIME_FMT "|R|DROP REQ\r\n", TIME_FMT_DATA(*time));
            return;
        }

        memcpy(forward, msg, size);
        -- forward->ttl;
        control_txbuffer_commit(size);
        return;
    }

    uint8_t n = update_node(msg->macaddr);
    uint8_t d = update_node(msg->dst_macaddr);
    if(n == SETTINGS_MAX_NODES || d == SETTINGS_MAX_NODES)
    {
        WARNING(TIME_FMT "|R|DROP REQ\r\n", TIME_FMT_DATA(*time));
        return;
    }

    if(!message_request_get_count(msg))
    {
        request.assignment[n].ttl = 0;
        return;
    }

    if(!request.assignment[n].ttl)
        queue_request(n, d, message_request_get_count(msg));

    else
        put_response_message(n);

    return;
}

void put_request_message(void)
{
    MessageRequest *msg =
        (MessageRequest *) control_txbuffer_get(sizeof(MessageRequest));

    if(!msg)
        return;

    if(!request.blocks_left)
    {
        request.blocks_left = random();
        uint8_t r = random() % neighbours_count();
        for(uint8_t n = 0; n < SETTINGS_MAX_NODES; ++ n)
        {
            if(!(neighbours.is_neighbour & _BV(n)))
                continue;

            if(!r)
            {
                request.destination = n;
                break;
            }

            -- r;
        }
    }

    msg->kind           = KIND_REQUEST;
    msg->macaddr        = device_macaddr;
    msg->dst_macaddr    = neighbours.node[request.destination].macaddr;
    msg->ttl            = SETTINGS_MAX_HOP;
    message_request_set_count(msg, request.blocks_left);
    control_txbuffer_commit(sizeof(MessageRequest));

    DEBUG(  TIME_FMT "|R|-REQ(-1,0x%04x,0x%04x,%u,%u)\r\n",
            (uint16_t) 0, (uint32_t) 0, msg->macaddr, msg->dst_macaddr,
            msg->ttl, message_request_get_count(msg));
}

inline
void queue_request(uint8_t source, uint8_t destination, uint16_t size)
{
    Request *queue = request.queue;
    for(int r = 0; r < request.queue_size; ++ r)
        if(queue[r].source == source &&
            queue[r].destination == destination)
        {
            queue[r].size = size;
            return;
        }

    if(request.queue_size >= SETTINGS_REQUEST_QUEUE_SIZE)
        return;

    queue = &request.queue[request.queue_size ++];
    queue->source       = source;
    queue->destination  = destination;
    queue->size         = size;
}

inline
void validate_request(void)
{
    for(uint8_t n = 0; n < SETTINGS_MAX_NODES; ++ n)
    {
        if(request.assignment[n].ttl)
        {
            -- request.assignment[n].ttl;

            if(!request.assignment[n].ttl)
                neighbours.node[n].color = 0xFF;
        }

        if( !request.assignment[n].ttl &&
            !request.queue_counter &&
            request.assignment[n].priority < 255)
            ++ request.assignment[n].priority;
    }

    ++ request.queue_counter;
}

#endif // __AVR__
#endif // __PROTOCOL_MESSAGES_REQUEST_H__
