#ifndef __MESSAGE_BACKOFF_STRUCT_H__
#define __MESSAGE_BACKOFF_STRUCT_H__

#include "protocol/structs.h"

typedef struct MessageBackoff
{
    MessageBackoffBase base;

    uint16_t macaddr;
    uint8_t payload[16];
} MessageBackoff;

#endif // __MESSAGE_BACKOFF_STRUCT_H__
