#ifndef __PROTOCOL_MESSAGES_NEIGHBOURS_H__
#define __PROTOCOL_MESSAGES_NEIGHBOURS_H__

#include <string.h>

#include "common.h"
#include "pingpong.h"
#include "synchronization.h"

//
// MessageNeighbours
//  Aktualni sąsiedzi urządzenia, informacja dla korzenia (BTS) pozwalająca mu
//  poznać strukturę sieci dla lepszego przydziału slotów
//  rozmiar: 1-161 bajtów
//
//  [3]     kind        - typ wiadomości = 0x6
//  [5]     count       - aktualna liczba sąsiadów
//  [16]    macaddr     - adres MAC nadawcy
//  [8]     ttl         - dla obsługi multi-hop
//  [*]     neighbour[] - dane sąsiadów (adres MAC, moc odbioru, ttl)
//

#define KIND_NEIGHBOURS 0x6

struct neighbour;

typedef struct neighbour
{
    uint16_t    macaddr;
    uint8_t     rssi;
    uint8_t     ttl;
} Neighbour;

typedef struct message_neighbours
{
    uint8_t kind    : 3;
    uint8_t count   : 5;

    uint16_t    macaddr;
    uint8_t     ttl;
    Neighbour   neighbour[];
} MessageNeighbours;

typedef const MessageNeighbours * const MessageNeighbours_cptr;

uint8_t message_neighbours_get_size(MessageNeighbours_cptr msg)
{
    return sizeof(MessageNeighbours) + msg->count * sizeof(Neighbour);
}


#ifdef __AVR__
////////////////////////////////////////////////////////////////////////////////

struct NeighboursData
{
    uint8_t     timer;
    Neighbour   neighbour[SETTINGS_MAX_NEIGHBOURS];
};

extern struct NeighboursData neighbours;


////////////////////////////////////////////////////////////////////////////////

void handle_neighbours( Time_cptr time, MessageNeighbours_cptr msg,
                        const uint8_t rssi);
void put_neighbours_message(void);
void update_neighbour(  Time_cptr time,
                        const uint16_t macaddr,
                        const uint8_t rssi);

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

    uint8_t size = message_neighbours_get_size(msg);
    if(pingpong.mode)
    {
        usart_push_block((uint8_t *) msg, size + 2); // +2 dla CRC
        DEBUG(  TIME_FMT "|U|-NS(-1,0x%04x,%u,%u)\r\n",
                TIME_FMT_DATA(*time), msg->macaddr, msg->ttl,
                msg->count);
    }

#ifndef STATIC_ROOT
    if(synchronization.root.macaddr != device_macaddr)
#else
    if(STATIC_ROOT != device_macaddr)
#endif
    {
        if(!msg->ttl)
            return;

        MessageNeighbours *forward = (MessageNeighbours *) txbuffer_get(size);

        if(!forward)
        {
            WARNING(TIME_FMT "|R|DROP NS\r\n", TIME_FMT_DATA(*time));
            return;
        }

        memcpy(forward, msg, size);
        -- forward->ttl;
        txbuffer_commit(size);
        return;
    }

    // TODO
}

void put_neighbours_message(void)
{
    Neighbour *neighbour = neighbours.neighbour;
    uint8_t valid_neighbours = neighbours_count();
    uint8_t size =  sizeof(MessageNeighbours) +
                    valid_neighbours * sizeof(Neighbour);

    MessageNeighbours *msg =
        (MessageNeighbours *) txbuffer_get(size);

    if(!msg)
        return;

    msg->kind       = KIND_NEIGHBOURS;
    msg->count      = valid_neighbours;
    msg->macaddr    = device_macaddr;
    msg->ttl        = SETTINGS_MAX_HOP;

    for(uint8_t n = 0, m = 0; n < SETTINGS_MAX_NEIGHBOURS; ++ n)
    {
        if(neighbour[n].ttl)
        {
            msg->neighbour[m].macaddr   = neighbour[n].macaddr;
            msg->neighbour[m].rssi      = neighbour[n].rssi;
            msg->neighbour[m].ttl       = neighbour[n].ttl;
            ++ m;
        }
    }

    txbuffer_commit(size);
    if(pingpong.mode)
    {
        usart_push_block((uint8_t *) msg, size + 2); // +2 dla CRC
        DEBUG(  TIME_FMT "|U|-NS(-1,0x%04x,%u,%u)\r\n",
                (uint16_t) 0, (uint32_t) 0, msg->macaddr, msg->ttl,
                msg->count);
    }

    DEBUG(  TIME_FMT "|R|-NS(-1,0x%04x,%u,%u)\r\n",
            (uint16_t) 0, (uint32_t) 0, msg->macaddr, msg->ttl,
            msg->count);
}

void update_neighbour(  Time_cptr time,
                        const uint16_t macaddr,
                        const uint8_t rssi)
{
    Neighbour *neighbour = neighbours.neighbour;
    uint8_t n;

    // Zaktualizuj istniejący wpis
    for(n = 0; n < SETTINGS_MAX_NEIGHBOURS; ++ n)
        if(neighbour[n].macaddr == macaddr)
        {
            neighbour[n].rssi   += rssi;
            neighbour[n].rssi   /= 2;
            neighbour[n].ttl    = SETTINGS_NEIGHBOUR_TTL;
            return;
        }

    // Dodaj nowy wpis
    for(n = 0; n < SETTINGS_MAX_NEIGHBOURS; ++ n)
        if(!neighbour[n].ttl)
        {
            neighbour[n].macaddr    = macaddr;
            neighbour[n].rssi       = rssi;
            neighbour[n].ttl        = SETTINGS_NEIGHBOUR_TTL;
            return;
        }

    // Brak miejsca na nowy wpis
    WARNING(TIME_FMT "| |Neighbours limit reached!", TIME_FMT_DATA(*time));
}

void validate_neighbours(void)
{
    ++ neighbours.timer;
    Neighbour *neighbour = neighbours.neighbour;
    for(uint8_t n = 0; n < SETTINGS_MAX_NEIGHBOURS; ++ n)
        if(neighbour[n].ttl)
            -- neighbour[n].ttl;
}

uint8_t neighbours_count(void)
{
    Neighbour *neighbour = neighbours.neighbour;
    uint8_t count = 0;
    for(uint8_t n = 0; n < SETTINGS_MAX_NEIGHBOURS; ++ n)
        if(neighbour[n].ttl)
            ++ count;

    return count;
}

#endif // __AVR__
#endif // __PROTOCOL_MESSAGES_NEIGHBOURS_H__
