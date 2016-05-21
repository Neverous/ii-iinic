#ifndef __MESSAGE_SYNCHRONIZATION_H__
#define __MESSAGE_SYNCHRONIZATION_H__

#include "../protocol/types.h"
#include "discovery.h"

typedef struct MessageSynchronization
{
    MessageSynchronization_base base;

    uint16_t macaddr;
    uint16_t root_macaddr;
    uint16_t seq_id;
    Time global_time;
} MessageSynchronization;

struct
{
    bool        add;
    Time        offset;
    uint16_t    seq_id;
    Time        last_sync;
} clock;

uint8_t send_sync_msg;
#if SYNCHRONIZATION_FAST_SYNC
bool    just_synced;
#endif // SYNCHRONIZATION_FAST_SYNC

void time_local_to_global(Time *global_time, Time_cptr local_time);
void time_global_to_local(Time *local_time, Time_cptr global_time);
void calculate_clock(   Time_cptr time, Time_cptr global_time,
                        const uint16_t seq_id);


void on_init_MessageSynchronization(__unused__ Time_cptr time,
                                    const uint8_t options)
{
    switch(options)
    {
        case MAIN_EVENT:
            send_sync_msg  = random() % SETTINGS_SYNCHRONIZATION_PERIOD;
#if SYNCHRONIZATION_FAST_SYNC
            just_synced     = false;
#endif // SYNCHRONIZATION_FAST_SYNC
            break;
    }
}

void on_frame_start_MessageSynchronization( Time_cptr frame_start,
                                            Time *frame_end,
                                            const uint8_t options)
{
    Time global_time; time_local_to_global(&global_time, frame_start);
    DEBUG(  "[" TIME_FMT "] clock_offset=%d:" TIME_FMT
            " seq_id=%u last_sync=" TIME_FMT " global_time=" TIME_FMT "\r\n",
            TIME_FMT_DATA(*frame_start), clock.add, TIME_FMT_DATA(clock.offset),
            clock.seq_id, TIME_FMT_DATA(clock.last_sync),
            TIME_FMT_DATA(global_time));

    switch(options)
    {
        case INITIALIZATION_EVENT:
            time_align(&global_time, SETTINGS_INITIALIZATION_FRAME_TIME);
            if((global_time.low / SETTINGS_INITIALIZATION_FRAME_TIME) % 2)
                iinic_led_off(IINIC_LED_GREEN);

            else
                iinic_led_on(IINIC_LED_GREEN);

            break;

        case TDMA_EVENT:
            time_align(&global_time, SETTINGS_TDMA_FRAME_TIME);
            if((global_time.low / SETTINGS_TDMA_FRAME_TIME) % 2)
                iinic_led_off(IINIC_LED_GREEN);

            else
                iinic_led_on(IINIC_LED_GREEN);

            break;
    }

    time_global_to_local(frame_end, &global_time);
}

void on_slot_start_MessageSynchronization(  Time_cptr slot_start,
                                            __unused__ Time *slot_end,
                                            const uint8_t options)
{
    if(options != TDMA_EVENT)
        return;

    if(iinic_mac == root.macaddr && root.ttl == 1)
        clock.seq_id = 0;

#if SYNCHRONIZATION_FAST_SYNC
    if(just_synced || (send_sync_msg % SETTINGS_SYNCHRONIZATION_PERIOD) == 0)
#else
    if((send_sync_msg % SETTINGS_SYNCHRONIZATION_PERIOD) == 0)
#endif // SYNCHRONIZATION_FAST_SYNC
    {
        if(iinic_mac == root.macaddr)
            ++ clock.seq_id;

        put_MessageSynchronization(slot_start, 0);
#if SYNCHRONIZATION_FAST_SYNC
        just_synced = false;
#endif // SYNCHRONIZATION_FAST_SYNC
    }

    send_sync_msg = (send_sync_msg + 1) % SETTINGS_SYNCHRONIZATION_PERIOD;
}

void on_slot_end_MessageSynchronization(__unused__ Time_cptr slot_end,
                                        __unused__ const uint8_t options)
{
}

void on_frame_end_MessageSynchronization(   __unused__ Time_cptr frame_end,
                                            __unused__ const uint8_t options)
{
}

void handle_MessageSynchronization(  Time_cptr time, const uint16_t rssi,
                                        MessageSynchronization *msg,
                                        const uint8_t options)
{
    Time global_time; time_local_to_global(&global_time, time);
    DEBUG(  "[" TIME_FMT "] Got synchronization message options=0x%02x rssi=%u"
            " macaddr=0x%04x root_macaddr=0x%04x seq_id=%u"
            " global_time=" TIME_FMT " current_global_time=" TIME_FMT "\r\n",
            TIME_FMT_DATA(*time), options, rssi, msg->macaddr,
            msg->root_macaddr, msg->seq_id, TIME_FMT_DATA(msg->global_time),
            TIME_FMT_DATA(global_time));

    if(msg->root_macaddr == iinic_mac ||                // Am I the root?
        msg->root_macaddr > root.macaddr ||             // Is it even root
        (msg->root_macaddr == root.macaddr &&           // If so, is it newer
         (int16_t) (msg->seq_id - clock.seq_id) <= 0))  // than before
        return;

    // Handle like discovery message
    handle_MessageDiscovery(time, rssi, (MessageDiscovery *) msg, options);

    calculate_clock(time, &msg->global_time, msg->seq_id);
    return;
}

uint8_t *write_MessageSynchronization(  Time_cptr time, uint8_t *buffer_start,
                                        uint8_t_cptr buffer_end,
                                        __unused__ uint8_t *ctx)
{
    if(buffer_start + sizeof(MessageSynchronization) > buffer_end)
        return NULL;

    MessageSynchronization *msg = (MessageSynchronization *) buffer_start;
    msg->base.kind              = KIND_SYNCHRONIZATION;
    msg->macaddr                = iinic_mac;
    msg->root_macaddr           = root.macaddr;
    msg->seq_id                 = clock.seq_id;
    time_local_to_global(&msg->global_time, time);
    return (uint8_t *) (msg + 1);
}

void time_local_to_global(Time *global_time, Time_cptr local_time)
{
    if(!local_time)
    {
        WARNING("[" TIME_FMT "] Invalid time for time_local_to_global\r\n",
                TIME_FMT_DATA(*local_time));
        return;
    }

    *global_time = *local_time;
    if(clock.add)
        time_add(global_time, &clock.offset);

    else
        time_sub(global_time, &clock.offset);

    DEBUG(  "[" TIME_FMT "] global_time=" TIME_FMT " local_time=" TIME_FMT
            "\r\n", TIME_FMT_DATA(*local_time), TIME_FMT_DATA(*global_time),
            TIME_FMT_DATA(*local_time));
}

void time_global_to_local(Time *local_time, Time_cptr global_time)
{
    if(!global_time)
    {
        WARNING("[" TIME_FMT "] Invalid time for time_global_to_local\r\n",
                TIME_FMT_DATA(*global_time));
        return;
    }

    *local_time = *global_time;
    if(clock.add)
        time_sub(local_time, &clock.offset);

    else
        time_add(local_time, &clock.offset);

    DEBUG(  "[" TIME_FMT "] local_time=" TIME_FMT " global_time=" TIME_FMT
            "\r\n", TIME_FMT_DATA(*local_time), TIME_FMT_DATA(*local_time),
            TIME_FMT_DATA(*global_time));
}

void calculate_clock(   Time_cptr time, Time_cptr global_time,
                        const uint16_t seq_id)
{

    uint64_t glob_time = ((1LL * global_time->high) << 32) +
                            global_time->low;

    uint64_t local_time = ((1LL * time->high) << 32) +
                            time->low;

    uint64_t offset = ((1LL * clock.offset.high) << 32) +
                            clock.offset.low;

    int64_t new_offset = glob_time - local_time;

    DEBUG(  "[" TIME_FMT "] previous clock_offset=%d:" TIME_FMT
            " seq_id=%u last_sync=" TIME_FMT "\r\n",
            TIME_FMT_DATA(*time), clock.add, TIME_FMT_DATA(clock.offset),
            clock.seq_id, TIME_FMT_DATA(clock.last_sync));

    if(new_offset < 0)
    {
        clock.add = false;
        new_offset = (-new_offset + offset) / 2;
    }

    else
    {
        clock.add = true;
        new_offset = (new_offset + offset) / 2;
    }

    clock.offset.low = new_offset;
    clock.offset.high = new_offset >> 32;

    clock.last_sync = *time;

    clock.seq_id = seq_id;

    NOTICE( "[" TIME_FMT "] new clock_offset=%d:" TIME_FMT
            " seq_id=%u last_sync=" TIME_FMT "\r\n",
            TIME_FMT_DATA(*time), clock.add, TIME_FMT_DATA(clock.offset),
            clock.seq_id, TIME_FMT_DATA(clock.last_sync));
}

#endif // __MESSAGE_SYNCHRONIZATION_H__
