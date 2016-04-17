#ifndef __MESSAGE_SYNCHRONIZATION_H__
#define __MESSAGE_SYNCHRONIZATION_H__

#include "../protocol/types.h"
#include "string.h"

typedef struct MessageSynchronization
{
    MessageSynchronization_base base;
    uint16_t root_macaddr;
    uint16_t seq_id;
    iinic_timing global_time;
} MessageSynchronization;

extern uint16_t lowest_macaddr;
extern uint8_t lowest_ttl;

iinic_timing clock_offset;
uint32_t clock_skew;
iinic_timing last_synchronization;
uint16_t seq_id;

void get_global_time(iinic_timing *global_time, iinic_timing_cptr *local_time);
void get_local_time(iinic_timing *local_time, iinic_timing_cptr *global_time);

void on_init_MessageSynchronization(_unused iinic_timing_cptr *time, _unused const uint8_t options)
{
}

void on_frame_start_MessageSynchronization(iinic_timing_cptr *frame_start, iinic_timing *frame_end, const uint8_t options)
{
    switch(options)
    {
        case INITIALIZATION_EVENT:
            return;
    }

    iinic_timing global_time; get_global_time(&global_time, frame_start);
    debug("[%lu] clock_offset=%ld clock_skew=%ld seq_id=%u last_synchronization=%lu global_time=%lu\r\n", *(uint32_t *) frame_start, *(int32_t *) &clock_offset, clock_skew, seq_id, *(uint32_t *) &last_synchronization, *(uint32_t *) &global_time);

    iinic_timing_align(&global_time, SETTINGS_TDMA_FRAME_TIME);
    get_local_time(frame_end, &global_time);
    if((*(uint32_t *) &global_time / SETTINGS_TDMA_FRAME_TIME) % 2)
        iinic_led_off(IINIC_LED_GREEN);

    else
        iinic_led_on(IINIC_LED_GREEN);
}

void on_slot_start_MessageSynchronization(iinic_timing_cptr *slot_start, _unused iinic_timing *slot_end, _unused const uint8_t options)
{
    if(options != TDMA_EVENT)
        return;

    static uint8_t fire = 0;
    if(fire % SETTINGS_FIRE_TTL == 0)
    {
        if(lowest_macaddr == iinic_mac)
            ++ seq_id;

        put_MessageSynchronization(slot_start, 0);
    }

    fire = (fire + 1) % SETTINGS_FIRE_TTL;
}

void on_slot_end_MessageSynchronization(_unused iinic_timing_cptr *slot_end, _unused const uint8_t options)
{
}

void on_frame_end_MessageSynchronization(_unused iinic_timing_cptr *frame_end, _unused const uint8_t options)
{
}

uint8_t handle_MessageSynchronization(iinic_timing_cptr *time, const uint16_t rssi, MessageSynchronization *msg, const uint8_t options)
{
    iinic_timing global_time; get_global_time(&global_time, time);
    debug("[%lu] Got synchronization message options=0x%02x rssi=%u root_macaddr=0x%04x seq_id=%u global_time=%lu curent_global_time=%lu\r\n", *(uint32_t *) time, options, rssi, msg->root_macaddr, msg->seq_id, *(uint32_t *) &msg->global_time, *(uint32_t *) &global_time);
    // TODO: add regression
    switch(options)
    {
        case TDMA_EVENT:
            if(msg->root_macaddr < lowest_macaddr)
                lowest_macaddr = msg->root_macaddr;

            else if(msg->root_macaddr > lowest_macaddr || (int16_t) (msg->seq_id - seq_id) <= 0)
                return 0;

            lowest_ttl = SETTINGS_ROOT_TTL;
            seq_id = msg->seq_id;
            last_synchronization = *time;
            clock_offset = msg->global_time;
            iinic_timing_sub(&clock_offset, time);
            break;

        case INITIALIZATION_EVENT:
            // Acts like discovery
            if(msg->root_macaddr < lowest_macaddr || !lowest_ttl)
            {
                lowest_macaddr = msg->root_macaddr;
                lowest_ttl = SETTINGS_ROOT_TTL;
            }

            seq_id = msg->seq_id;
            last_synchronization = *time;
            clock_offset = msg->global_time;
            iinic_timing_sub(&clock_offset, time);
            break;
    }

    return 0;
}

uint8_t *write_MessageSynchronization(iinic_timing_cptr *time, uint8_t *buffer_start, const uint8_t const *buffer_end, _unused uint8_t *ctx)
{
    if(buffer_start + sizeof(MessageSynchronization) > buffer_end)
        return 0;

    MessageSynchronization *msg = (MessageSynchronization *) buffer_start;
    msg->base.kind              = KIND_SYNCHRONIZATION;
    msg->root_macaddr           = lowest_macaddr;
    msg->seq_id                 = seq_id;
    get_global_time(&msg->global_time, time);
    return (uint8_t *) (msg + 1);
}

void get_global_time(iinic_timing *global_time, iinic_timing_cptr *local_time)
{
    if(!local_time)
        return;

    *global_time = *local_time;
    iinic_timing_add(global_time, &clock_offset);
    // TODO: skew / drift => + (local_time - last_synchronization) / clock_skew
}

void get_local_time(iinic_timing *local_time, iinic_timing_cptr *global_time)
{
    if(!global_time)
        return;

    *local_time = *global_time;
    iinic_timing_sub(local_time, &clock_offset);
    // TODO: skew / drift => - (local_time - last_synchronization) / clock_skew
}

#endif // __MESSAGE_SYNCHRONIZATION_H__
