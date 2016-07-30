#ifndef __PROTOCOL_MESSAGES_REQUEST_STRUCTS_H__
#define __PROTOCOL_MESSAGES_REQUEST_STRUCTS_H__

#include "common.h"

typedef struct assignment
{
    uint8_t     ttl;
    uint8_t     priority;
    uint16_t    slotmask;
} Assignment;

#ifdef __AVR__
////////////////////////////////////////////////////////////////////////////////

typedef struct request
{
    uint8_t source;
    uint8_t destination;
    uint16_t size;
} Request;

struct RequestData
{
    uint16_t    bytes_left;
    uint8_t     destination;
    Assignment  assignment[SETTINGS_MAX_NODES];
    uint8_t     queue_counter;
    uint8_t     queue_size;
    Request     queue[SETTINGS_REQUEST_QUEUE_SIZE];
};

extern struct RequestData request;

#endif // __AVR__
#endif // __PROTOCOL_MESSAGES_REQUEST_STRUCTSH__
