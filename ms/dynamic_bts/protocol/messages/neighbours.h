#ifndef __PROTOCOL_MESSAGES_NEIGHBOURS_H__
#define __PROTOCOL_MESSAGES_NEIGHBOURS_H__

#include <string.h>

#include "common.h"
#include "debug.h"
#include "neighbours_structs.h"
#include "ping.h"
#include "synchronization.h"

//
// MessageNeighbours
//  Aktualni sąsiedzi urządzenia, informacja dla korzenia (BTS) pozwalająca mu
//  poznać strukturę sieci dla lepszego przydziału slotów
//  rozmiar: 4-132 bajtów
//
//  [3]     kind        - typ wiadomości = 0x1
//  [5]     count       - aktualna liczba sąsiadów
//  [16]    macaddr     - adres MAC nadawcy
//  [8]     ttl         - dla obsługi multi-hop
//  [*]     neighbour[] - dane sąsiadów (adres MAC, moc odbioru, ttl)
//

#define KIND_NEIGHBOURS 0x1

struct neighbour;

typedef struct neighbour_info
{
    uint16_t    macaddr;
    uint8_t     rssi;
    uint8_t     ttl;
} NeighbourInfo;

typedef struct message_neighbours
{
    uint8_t kind    : 3;
    uint8_t count   : 5;

    uint16_t        macaddr;
    uint8_t         ttl;
    NeighbourInfo   neighbour[];
} MessageNeighbours;

typedef const MessageNeighbours * const MessageNeighbours_cptr;

inline
uint8_t message_neighbours_get_size(MessageNeighbours_cptr msg)
{
    return sizeof(MessageNeighbours) + msg->count * sizeof(NeighbourInfo);
}


#ifdef __AVR__
////////////////////////////////////////////////////////////////////////////////

void handle_neighbours( Time_cptr time, MessageNeighbours_cptr msg,
                        const uint8_t rssi);
void put_neighbours_message(void);
void update_neighbour(  Time_cptr time,
                        const uint16_t macaddr,
                        const uint8_t rssi);
uint8_t update_node(const uint16_t macaddr);
uint8_t update_edge(const uint8_t source, const uint8_t destination,
                    const uint8_t rssi, const uint8_t ttl);

void validate_neighbours(void);
uint8_t neighbours_count(void);


////////////////////////////////////////////////////////////////////////////////

void handle_neighbours( Time_cptr time, MessageNeighbours_cptr msg,
                        const uint8_t rssi)
{
    DEBUG(  TIME_FMT "|R|+NS(%u,0x%04x,%u,%u)\r\n",
            TIME_FMT_DATA(*time), rssi, msg->macaddr, msg->ttl, msg->count);

    if(msg->macaddr == device_macaddr)
        return;

    update_neighbour(time, msg->macaddr, rssi);
    uint8_t size = message_neighbours_get_size(msg);
    _MODE_MONITOR({
        usart_push_block((uint8_t *) msg, size + 2, true); // +2 dla CRC
        DEBUG(  TIME_FMT "|U|-NS(0x%04x,%u,%u)\r\n",
                TIME_FMT_DATA(*time), msg->macaddr, msg->ttl,
                msg->count);
    });

    uint8_t src = update_node(msg->macaddr);
    if(src != SETTINGS_MAX_NODES)
        for(uint8_t n = 0; n < msg->count; ++ n)
        {
            uint8_t dst = update_node(msg->neighbour[n].macaddr);
            if(dst != SETTINGS_MAX_NODES)
                update_edge(src, dst,
                            msg->neighbour[n].rssi, msg->neighbour[n].ttl);
        }


#ifndef STATIC_ROOT
    if(synchronization.root.macaddr != device_macaddr)
#else
    if(STATIC_ROOT != device_macaddr)
#endif
    {
        if(!msg->ttl)
            return;

        MessageNeighbours *forward =
            (MessageNeighbours *) control_txbuffer_get(size);

        if(!forward)
        {
            WARNING(TIME_FMT "|R|DROP NS\r\n", TIME_FMT_DATA(*time));
            return;
        }

        memcpy(forward, msg, size);
        -- forward->ttl;
        control_txbuffer_commit(size);
    }
}

void put_neighbours_message(void)
{
    uint8_t valid_neighbours = neighbours_count();
    uint8_t size =  sizeof(MessageNeighbours) +
                    valid_neighbours * sizeof(NeighbourInfo);

    MessageNeighbours *msg =
        (MessageNeighbours *) control_txbuffer_get(size);

    if(!msg)
        return;

    msg->kind       = KIND_NEIGHBOURS;
    msg->count      = valid_neighbours;
    msg->macaddr    = device_macaddr;
    msg->ttl        = SETTINGS_MAX_HOP;

    Node *node = neighbours.node;
    Edge *edge = neighbours.edge;
    for(uint8_t e = 0, m = 0; e < SETTINGS_MAX_EDGES; ++ e)
    {
        if(!edge[e].ttl)
            continue;

        if(edge[e].source && edge[e].destination)
            continue;

        msg->neighbour[m].macaddr   =
            node[edge[e].source | edge[e].destination].macaddr;

        msg->neighbour[m].rssi      = edge[e].rssi;
        msg->neighbour[m].ttl       = edge[e].ttl;
        ++ m;
    }

    control_txbuffer_commit(size);
    _MODE_MONITOR({
        usart_push_block((uint8_t *) msg, size + 2, true); // +2 dla CRC
        DEBUG(  TIME_NULL "|U|-NS(0x%04x,%u,%u)\r\n",
                msg->macaddr, msg->ttl,
                msg->count);
    });

    DEBUG(  TIME_NULL "|R|-NS(0x%04x,%u,%u)\r\n",
            msg->macaddr, msg->ttl,
            msg->count);
}

void update_neighbour(  Time_cptr time,
                        const uint16_t macaddr,
                        const uint8_t rssi)
{
    uint8_t n = update_node(macaddr);
    if(n == SETTINGS_MAX_NODES)
    {
        WARNING(    TIME_FMT "| |Nodes limit reached!",
                    TIME_FMT_DATA(*time));
        return;
    }

    uint8_t e = update_edge(0, n, rssi, SETTINGS_NEIGHBOUR_TTL);
    if(e == SETTINGS_MAX_EDGES)
    {
        WARNING(    TIME_FMT "| |Edges limit reached!",
                    TIME_FMT_DATA(*time));
        return;
    }
}

inline
void validate_neighbours(void)
{
    Edge *edge = neighbours.edge;
    neighbours.is_neighbour = 0;
    for(uint8_t e = 0; e < SETTINGS_MAX_EDGES; ++ e)
    {
        if(edge[e].ttl)
            -- edge[e].ttl;

        if(!edge[e].ttl)
            continue;

        if(!edge[e].source)
            neighbours.is_neighbour |= _BV(edge[e].destination);

        if(!edge[e].destination)
            neighbours.is_neighbour |= _BV(edge[e].source);
    }

    Node *node = neighbours.node;
    for(uint8_t n = 0; n < SETTINGS_MAX_NODES; ++ n)
    {
        if(node[n].ttl)
            -- node[n].ttl;
    }
}

uint8_t update_node(const uint16_t macaddr)
{
    if(macaddr == iinic_mac || macaddr == device_macaddr)
        return 0;

    Node *node = neighbours.node;
    uint8_t n = 1;

    while(n < SETTINGS_MAX_NODES && node[n].macaddr != macaddr)
        ++ n;

    if(n == SETTINGS_MAX_NODES)
    {
        n = 1;
        while(n < SETTINGS_MAX_NODES && node[n].macaddr)
            ++ n;

        if(n == SETTINGS_MAX_NODES)
        {
            n = 1;
            while(n < SETTINGS_MAX_NODES && node[n].ttl)
                ++ n;
        }

        if(n != SETTINGS_MAX_NODES)
        {
            node[n].macaddr = macaddr;
            node[n].color   = 0xFF;
        }
    }

    if(n != SETTINGS_MAX_NODES)
        node[n].ttl     = 255;

    return n;
}

uint8_t update_edge(const uint8_t source, const uint8_t destination,
                    const uint8_t rssi, const uint8_t ttl)
{
    Edge *edge = neighbours.edge;
    uint8_t e;
    for(e = 0; e < SETTINGS_MAX_EDGES; ++ e)
    {
        if(!edge[e].ttl)
            continue;

        if(edge[e].source == source && edge[e].destination == destination)
            break;

        if(edge[e].source == destination && edge[e].destination == source)
            break;
    }

    if(e == SETTINGS_MAX_EDGES)
    {
        e = 0;
        while(e < SETTINGS_MAX_EDGES && edge[e].ttl)
            ++ e;

        if(e != SETTINGS_MAX_EDGES)
        {
            edge[e].source = source;
            edge[e].destination = destination;
        }
    }

    if(e != SETTINGS_MAX_EDGES)
    {
        edge[e].rssi = ((uint16_t) edge[e].rssi + rssi) / 2;
        edge[e].ttl = max(edge[e].ttl, ttl);
    }

    return e;
}

inline
uint8_t neighbours_count(void)
{
    return __builtin_popcount(neighbours.is_neighbour);
}

#endif // __AVR__
#endif // __PROTOCOL_MESSAGES_NEIGHBOURS_H__
