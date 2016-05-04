#ifndef __MESSAGE_NEIGHBOURS_STRUCT_H__
#define __MESSAGE_NEIGHBOURS_STRUCT_H__

#include "protocol/structs.h"

extern uint8_t neighbours_ttl;

typedef struct MessageNeighbours
{
    MessageNeighboursBase base;

    uint16_t macaddr;
} MessageNeighbours;

#endif // __MESSAGE_NEIGHBOURS_STRUCT_H__
