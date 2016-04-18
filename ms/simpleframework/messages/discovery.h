#ifndef __MESSAGE_DISCOVERY_H__
#define __MESSAGE_DISCOVERY_H__

#include <stdlib.h>

#include "../protocol/types.h"

typedef struct MessageDiscovery
{
    MessageDiscovery_base base;

    uint16_t macaddr;
    uint16_t root_macaddr;
} MessageDiscovery;

typedef struct Neighbour
{
    uint16_t    macaddr;
    uint8_t     ttl;
} Neighbour;

Neighbour neighbour[SETTINGS_MAX_NEIGHBOURS];
Neighbour root;

uint8_t send_discovery_msg;

void check_neighbours(Time_cptr *time);
void update_neighbour(Time_cptr *time, uint16_t macaddr, uint8_t ttl);

void on_init_MessageDiscovery(Time_cptr *time, const uint8_t options)
{
    switch(options)
    {
        case MAIN_EVENT:
            root.macaddr        = iinic_mac;
            root.ttl            = SETTINGS_ROOT_TTL;
            if(SETTINGS_DISCOVERY_PERIOD > 1)
                send_discovery_msg  = random() % SETTINGS_DISCOVERY_PERIOD;

            break;

        case TDMA_EVENT:
            if(root.macaddr == iinic_mac)
            {
                NOTICE( "[" TIME_FMT "] I am the root\r\n",
                        TIME_FMT_DATA(*time));

                root.ttl = SETTINGS_ROOT_TIME;
            }

            break;
    }
}

void on_frame_start_MessageDiscovery(   Time_cptr *frame_start,
                                        _unused Time *frame_deadline,
                                        const uint8_t options)
{
    switch(options)
    {
        case INITIALIZATION_EVENT:
            return;

        case TDMA_EVENT:
            if(root.ttl)
                -- root.ttl;

            else if(root.macaddr != iinic_mac)
            {
                NOTICE( "[" TIME_FMT "] I am the root\r\n",
                        TIME_FMT_DATA(*frame_start));

                root.macaddr = iinic_mac;
                root.ttl = SETTINGS_ROOT_TIME;
            }

            break;
    }

    DEBUG(  "[" TIME_FMT "] current root: macaddr=0x%04x ttl=%u\r\n",
            TIME_FMT_DATA(*frame_start), root.macaddr, root.ttl);

    check_neighbours(frame_start);
}

void on_slot_start_MessageDiscovery(Time_cptr *slot_start,
                                    _unused Time *slot_end,
                                    const uint8_t options)
{
    switch(options)
    {
        case INITIALIZATION_EVENT:
            put_MessageDiscovery(slot_start, 0);
            break;

        case TDMA_EVENT:
            if(SETTINGS_DISCOVERY_PERIOD > 1)
            {
                if(send_discovery_msg % SETTINGS_DISCOVERY_PERIOD == 0)
                {
                    put_MessageDiscovery(slot_start, 0);
                }

                send_discovery_msg =
                    (send_discovery_msg + 1) % SETTINGS_DISCOVERY_PERIOD;
            }

            break;
    }
}

void on_slot_end_MessageDiscovery(  _unused Time_cptr *slot_end,
                                    _unused const uint8_t options)
{
}

void on_frame_end_MessageDiscovery( _unused Time_cptr *slot_end,
                                    _unused const uint8_t options)
{
}

uint8_t handle_MessageDiscovery(Time_cptr *time, const uint16_t rssi,
                                MessageDiscovery *msg, const uint8_t options)
{
    NOTICE( "[" TIME_FMT "] Got discovery message options=0x%02x rssi=%u "
            " macaddr=0x%04x root_macaddr=0x%04x\r\n", TIME_FMT_DATA(*time),
            options, rssi, msg->macaddr, msg->root_macaddr);

    if(root.macaddr > msg->root_macaddr &&
        (root.macaddr != iinic_mac || root.ttl == 0))
    {
        NOTICE( "[" TIME_FMT "] Found new root macaddr=0x%04x\r\n",
                TIME_FMT_DATA(*time), msg->root_macaddr);

        root.macaddr = msg->root_macaddr;
        root.ttl = SETTINGS_ROOT_TTL;
    }

    update_neighbour(time, msg->macaddr, SETTINGS_ROOT_TTL);
    return 0;
}

uint8_t *write_MessageDiscovery(_unused Time_cptr *time, uint8_t *buffer_start,
                                const uint8_t const *buffer_end,
                                _unused uint8_t *ctx)
{
    if(buffer_start + sizeof(MessageDiscovery) > buffer_end)
        return 0;

    MessageDiscovery *msg   = (MessageDiscovery *) buffer_start;
    msg->base.kind          = KIND_DISCOVERY;
    msg->macaddr            = iinic_mac;
    msg->root_macaddr       = root.macaddr;
    return (uint8_t *) (msg + 1);
}

void check_neighbours(Time_cptr *time)
{
    for(uint8_t n = 0; n < SETTINGS_MAX_NEIGHBOURS; ++ n)
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

void update_neighbour(Time_cptr *time, uint16_t macaddr, uint8_t ttl)
{
    for(uint8_t n = 0; n < SETTINGS_MAX_NEIGHBOURS; ++ n)
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

    for(uint8_t n = 0; n < SETTINGS_MAX_NEIGHBOURS; ++ n)
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

#endif // __MESSAGE_DISCOVERY_H__
