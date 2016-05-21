#ifndef __MESSAGE_DISCOVERY_NEIGHBOURS_H__
#define __MESSAGE_DISCOVERY_NEIGHBOURS_H__

#include <stdlib.h>

const uint8_t DISCOVERY_NO_ROOT = 128;

typedef struct Neighbour
{
    uint16_t    macaddr;
    uint8_t     ttl;
} Neighbour;

extern Neighbour neighbour[SETTINGS_MAX_SENSORS];

uint8_t get_neighbours_count(void);
void check_neighbours(Time_cptr time);
void update_neighbour(Time_cptr time, uint16_t macaddr, uint8_t ttl);

uint8_t get_neighbours_count(void)
{
    uint8_t count = 0;
    for(uint8_t n = 0; n < SETTINGS_MAX_SENSORS; ++ n)
    {
        if(!neighbour[n].ttl)
            continue;

        ++ count;
    }

    return count;
}

void check_neighbours(Time_cptr time)
{
    for(uint8_t n = 0; n < SETTINGS_MAX_SENSORS; ++ n)
    {
        if(!neighbour[n].ttl)
            continue;

        -- neighbour[n].ttl;
        if(!neighbour[n].ttl)
        {
            NOTICE( "[" TIME_FMT "] dropping neighbour 0x%04x\r\n",
                    TIME_FMT_DATA(*time), neighbour[n].macaddr);
        }
    }
}

void update_neighbour(Time_cptr time, uint16_t macaddr, uint8_t ttl)
{
    for(uint8_t n = 0; n < SETTINGS_MAX_SENSORS; ++ n)
    {
        if(!neighbour[n].ttl)
            continue;

        if(neighbour[n].macaddr == macaddr)
        {
            neighbour[n].ttl = ttl;
            return;
        }
    }

    // NEW NEIGHBOUR
    NOTICE( "[" TIME_FMT "] Adding new neighbour 0x%04x\r\n",
            TIME_FMT_DATA(*time), macaddr);

    for(uint8_t n = 0; n < SETTINGS_MAX_SENSORS; ++ n)
    {
        if(neighbour[n].ttl)
            continue;

        neighbour[n].macaddr = macaddr;
        neighbour[n].ttl = ttl;
        return;
    }

    WARNING("[" TIME_FMT "] Maximum number of neighbours found!\r\n",
            TIME_FMT_DATA(*time));
}

#endif // __MESSAGE_DISCOVERY_NEIGHBOURS_H__
