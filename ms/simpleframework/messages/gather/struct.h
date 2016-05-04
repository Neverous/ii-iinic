#ifndef __MESSAGE_GATHER_STRUCT_H__
#define __MESSAGE_GATHER_STRUCT_H__

#include "protocol/structs.h"

typedef struct MessageGather
{
    MessageGatherBase base;

    uint16_t macaddr;
    uint8_t payload[];
} MessageGather;

#endif // __MESSAGE_GATHER_STRUCT_H__
