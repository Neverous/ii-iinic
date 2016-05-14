#ifndef __MESSAGE_BACKOFF_STRUCT_H__
#define __MESSAGE_BACKOFF_STRUCT_H__

#include "protocol/structs.h"

typedef struct MessageBackoff
{
    MessageBackoffBase base;

    uint8_t seq_id;
    uint16_t src_macaddr;
    uint16_t dest_macaddr;
    uint8_t payload[SETTINGS_BACKOFF_PAYLOAD_SIZE];
} MessageBackoff;

typedef struct MessageBackoffAck
{
    MessageBackoffAckBase base;

    uint16_t src_macaddr;
    uint16_t dest_macaddr;
} MessageBackoffAck;

#endif // __MESSAGE_BACKOFF_STRUCT_H__
