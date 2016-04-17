#ifndef __MESSAGE_NEIGHBOURS_H__
#define __MESSAGE_NEIGHBOURS_H__

#include "../protocol/types.h"

uint8_t neighbours_ttl;

typedef struct MessageNeighbours
{
    MessageNeighbours_base base;
} MessageNeighbours;

void on_init_MessageNeighbours(_unused iinic_timing_cptr *time, _unused const uint8_t options)
{
}

void on_frame_start_MessageNeighbours(_unused iinic_timing_cptr *frame_start, _unused iinic_timing *frame_deadline, _unused const uint8_t options)
{
    if(!neighbours_ttl)
        return;

    iinic_led_on(IINIC_LED_RED);
}

void on_slot_start_MessageNeighbours(_unused iinic_timing_cptr *slot_start, _unused iinic_timing *slot_end, _unused const uint8_t options)
{
    if(iinic_read_button() && neighbours_ttl < SETTINGS_NEIGHBOURS_TTL / 2)
    {
        neighbours_ttl = SETTINGS_NEIGHBOURS_TTL;
        put_MessageNeighbours(slot_start, 0);
    }
}

void on_slot_end_MessageNeighbours(_unused iinic_timing_cptr *slot_end, _unused const uint8_t options)
{
}

void on_frame_end_MessageNeighbours(_unused iinic_timing_cptr *slot_end, _unused const uint8_t options)
{
    if(!neighbours_ttl)
        return;

    -- neighbours_ttl;
    iinic_led_off(IINIC_LED_RED);
}

uint8_t handle_MessageNeighbours(iinic_timing_cptr *time, const uint16_t rssi, _unused MessageNeighbours *msg, const uint8_t options)
{
    debug("[%lu] Got neighbours message options=0x%02x rssi=%u\r\n", *(uint32_t *) time, options, rssi);
    neighbours_ttl = SETTINGS_NEIGHBOURS_TTL;
    return 0;
}

uint8_t *write_MessageNeighbours(_unused iinic_timing_cptr *time, uint8_t *buffer_start, const uint8_t const *buffer_end, _unused uint8_t *ctx)
{
    if(buffer_start + sizeof(MessageNeighbours) > buffer_end)
        return 0;

    MessageNeighbours *msg  = (MessageNeighbours *) buffer_start;
    msg->base.kind          = KIND_NEIGHBOURS;
    return (uint8_t *) (msg + 1);
}


#endif // __MESSAGE_NEIGHBOURS_H__
