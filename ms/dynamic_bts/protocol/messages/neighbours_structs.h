#ifndef __PROTOCOL_MESSAGES_NEIGHBOURS_STRUCTS_H__
#define __PROTOCOL_MESSAGES_NEIGHBOURS_STRUCTS_H__

#include "common.h"

#ifdef __AVR__
////////////////////////////////////////////////////////////////////////////////

typedef struct node
{
    uint16_t    macaddr;
    uint8_t     color;
    uint8_t     ttl;
} Node;

typedef struct edge
{
    uint8_t rssi;
    uint8_t ttl;
    uint8_t source      : 4;
    uint8_t destination : 4;
} Edge;

struct NeighboursData
{
    uint16_t    is_neighbour;
    Node        node[SETTINGS_MAX_NODES];
    Edge        edge[SETTINGS_MAX_EDGES];
};

extern struct NeighboursData neighbours;

#endif // __AVR__
#endif // __PROTOCOL_MESSAGES_NEIGHBOURS_STRUCTS_H__
