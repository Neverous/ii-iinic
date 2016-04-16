#ifndef __MESSAGE_DISCOVERY_H__
#define __MESSAGE_DISCOVERY_H__

#include "../protocol/types.h"

typedef struct MessageDiscovery
{
    MessageDiscovery_base base;
    uint16_t lowest_macaddr;
} MessageDiscovery;

uint16_t lowest_macaddr;
uint8_t lowest_ttl;

void on_init_MessageDiscovery(uint8_t options)
{
    switch(options)
    {
        case MAIN_EVENT:
            lowest_macaddr = iinic_mac;
            lowest_ttl = SETTINGS_ROOT_TTL;
            break;

        case TDMA_EVENT:
            if(lowest_macaddr == iinic_mac)
                debug("[?] I am the master\r\n");

            break;
    }
}

void on_frame_start_MessageDiscovery(_unused iinic_timing *time, uint8_t options)
{
    switch(options)
    {
        case TDMA_EVENT:
            if(lowest_ttl)
                -- lowest_ttl;

            else
                lowest_macaddr = iinic_mac;

            break;
    }

    debug("[%lu] current master: mac=0x%04x ttl=%u\r\n", *(uint32_t *)time, lowest_macaddr, lowest_ttl);
}

void on_slot_start_MessageDiscovery(_unused iinic_timing *time, uint8_t options)
{
    switch(options)
    {
        case INITIALIZATION_EVENT:
            put_MessageDiscovery();
            break;
    }
}

void on_slot_end_MessageDiscovery(_unused const iinic_timing const *time, _unused uint8_t options)
{
}

void on_frame_end_MessageDiscovery(_unused const iinic_timing const *time, _unused uint8_t options)
{
}

uint8_t handle_MessageDiscovery(uint16_t rssi, MessageDiscovery *msg, uint8_t options)
{
    debug("[?] Got discovery message rssi=%u lowest_macaddr=0x%04x options=0x%02x\r\n", rssi, msg->lowest_macaddr, options);
    if(options != INITIALIZATION_EVENT)
        return 0;

    if(lowest_macaddr > msg->lowest_macaddr || !lowest_ttl)
    {
        lowest_macaddr = msg->lowest_macaddr;
        lowest_ttl = SETTINGS_ROOT_TTL;
    }

    return 0;
}

uint8_t *write_MessageDiscovery(uint8_t *buffer_start, const uint8_t const *buffer_end)
{
    if(buffer_start + sizeof(MessageDiscovery) > buffer_end)
        return 0;

    MessageDiscovery *msg = (MessageDiscovery *) buffer_start;
    msg->base.kind = KIND_DISCOVERY;
    msg->lowest_macaddr = lowest_macaddr;
    return (uint8_t *) (msg + 1);
}


#endif // __MESSAGE_DISCOVERY_H__
