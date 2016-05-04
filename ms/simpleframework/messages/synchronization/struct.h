#ifndef __MESSAGE_SYNCHRONIZATION_STRUCT_H__
#define __MESSAGE_SYNCHRONIZATION_STRUCT_H__

#include "protocol/structs.h"

typedef struct MessageSynchronization
{
    MessageSynchronizationBase base;

    uint16_t macaddr;
    uint16_t root_macaddr;
    uint16_t seq_id;
    Time global_time;
} MessageSynchronization;

#endif // __MESSAGE_SYNCHRONIZATION_STRUCT_H__
