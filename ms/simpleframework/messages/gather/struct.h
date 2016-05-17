#ifndef __MESSAGE_GATHER_STRUCT_H__
#define __MESSAGE_GATHER_STRUCT_H__

#include "protocol/structs.h"
#include "messages/backoff/stats.h"

typedef struct IndexedData
{
    union
    {
        uint8_t     index;
        struct
        {
            uint8_t index_high  :4;
            uint8_t index_low   :4;
        };
    };

    uint16_t    value;
} IndexedData;

typedef struct MessageGather
{
    MessageGatherBase base;

    uint8_t     ttl;
    uint16_t    macaddr;
    IndexedData stats[];
} MessageGather;

typedef struct MessageGatherRequest
{
    MessageGatherRequestBase base;

    uint8_t ttl;
} MessageGatherRequest;

#endif // __MESSAGE_GATHER_STRUCT_H__
