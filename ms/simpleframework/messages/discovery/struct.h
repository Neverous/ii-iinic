#ifndef __MESSAGE_DISCOVERY_STRUCT_H__
#define __MESSAGE_DISCOVERY_STRUCT_H__

#include "protocol/structs.h"

typedef struct MessageDiscovery
{
    MessageDiscoveryBase base;

    uint16_t macaddr;
    uint16_t root_macaddr;
} MessageDiscovery;

#endif // __MESSAGE_DISCOVERY_STRUCT_H__
