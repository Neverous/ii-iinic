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
    int32_t     skew;
    uint16_t    seq_id;
    Time        last_sync;
} clock;

typedef enum
{
    STATE_INVALID   = 0,
    STATE_VALID     = 1,
} SyncPointState;

struct
{
    SyncPointState  state;
    Time            local_time;
    Time            global_time;
    uint16_t        seq_id;
    uint16_t        root_macaddr;
} sync_point[SETTINGS_SYNCHRONIZATION_POINTS];
uint8_t valid_sync_points;

uint8_t send_sync_msg;

void time_local_to_global(Time *global_time, Time_cptr *local_time);
void time_global_to_local(Time *local_time, Time_cptr *global_time);
void add_sync_point(Time_cptr *local_time, Time_cptr *global_time,
                    const uint16_t seq_id, const uint16_t root_macaddr);

void validate_sync_points(void);
void calculate_clock(Time_cptr *time);


void on_init_MessageSynchronization(_unused Time_cptr *time,
                                    const uint8_t options)
{
    switch(options)
    {
        case MAIN_EVENT:
            send_sync_msg  = random() % SETTINGS_SYNCHRONIZATION_PERIOD;
            break;
    }
}

void on_frame_start_MessageSynchronization( Time_cptr *frame_start,
                                            Time *frame_end,
                                            const uint8_t options)
{
    Time global_time; time_local_to_global(&global_time, frame_start);
    DEBUG(  "[" TIME_FMT "] clock_offset=%d:" TIME_FMT " clock_skew=%ld"
            " seq_id=%u last_sync=" TIME_FMT " global_time=" TIME_FMT "\r\n",
            TIME_FMT_DATA(*frame_start), clock.add, TIME_FMT_DATA(clock.offset),
            clock.skew, clock.seq_id, TIME_FMT_DATA(clock.last_sync),
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
    validate_sync_points();
}

void on_slot_start_MessageSynchronization(  Time_cptr *slot_start,
                                            _unused Time *slot_end,
                                            const uint8_t options)
{
    if(options != TDMA_EVENT)
        return;

    if(iinic_mac != root.macaddr &&
        valid_sync_points < SETTINGS_SYNCHRONIZATION_POINTS / 2)
        return;

    if(iinic_mac == root.macaddr && root.ttl == 1)
        clock.seq_id = 0;

    if(send_sync_msg % SETTINGS_SYNCHRONIZATION_PERIOD == 0)
    {
        if(iinic_mac == root.macaddr)
            ++ clock.seq_id;

        put_MessageSynchronization(slot_start, 0);
    }

    send_sync_msg = (send_sync_msg + 1) % SETTINGS_SYNCHRONIZATION_PERIOD;
}

void on_slot_end_MessageSynchronization(_unused Time_cptr *slot_end,
                                        _unused const uint8_t options)
{
}

void on_frame_end_MessageSynchronization(   _unused Time_cptr *frame_end,
                                            _unused const uint8_t options)
{
}

uint8_t handle_MessageSynchronization(  Time_cptr *time, const uint16_t rssi,
                                        MessageSynchronization *msg,
                                        const uint8_t options)
{
    Time global_time; time_local_to_global(&global_time, time);
    NOTICE( "[" TIME_FMT "] Got synchronization message options=0x%02x rssi=%u"
            " macaddr=0x%04x root_macaddr=0x%04x seq_id=%u"
            " global_time=" TIME_FMT " current_global_time=" TIME_FMT "\r\n",
            TIME_FMT_DATA(*time), options, rssi, msg->macaddr,
            msg->root_macaddr, msg->seq_id, TIME_FMT_DATA(msg->global_time),
            TIME_FMT_DATA(global_time));

    if(msg->root_macaddr == iinic_mac ||                // Am I the root?
        msg->root_macaddr > root.macaddr ||             // Is it even root
        (msg->root_macaddr == root.macaddr &&           // If so, is it newer
         (int16_t) (msg->seq_id - clock.seq_id) <= 0))  // than before
        return 0;

    // Handle like discovery message
    handle_MessageDiscovery(time, rssi, (MessageDiscovery *) msg, options);

    add_sync_point(time, &msg->global_time, msg->seq_id, msg->root_macaddr);
    calculate_clock(time);
    return 0;
}

uint8_t *write_MessageSynchronization(  Time_cptr *time, uint8_t *buffer_start,
                                        const uint8_t const *buffer_end,
                                        _unused uint8_t *ctx)
{
    if(buffer_start + sizeof(MessageSynchronization) > buffer_end)
        return 0;

    MessageSynchronization *msg = (MessageSynchronization *) buffer_start;
    msg->base.kind              = KIND_SYNCHRONIZATION;
    msg->macaddr                = iinic_mac;
    msg->root_macaddr           = root.macaddr;
    msg->seq_id                 = clock.seq_id;
    time_local_to_global(&msg->global_time, time);
    return (uint8_t *) (msg + 1);
}

void time_local_to_global(Time *global_time, Time_cptr *local_time)
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

    if(clock.skew)
    {
        Time skew_diff = *local_time;
        time_sub(&skew_diff, &clock.last_sync);
        time_add_i32(global_time, 100 * skew_diff.low / clock.skew);
        skew_diff.low = 0;
        time_add(global_time, &skew_diff);
    }

    DEBUG(  "[" TIME_FMT "] global_time=" TIME_FMT " local_time=" TIME_FMT
            "\r\n", TIME_FMT_DATA(*local_time), TIME_FMT_DATA(*global_time),
            TIME_FMT_DATA(*local_time));
}

void time_global_to_local(Time *local_time, Time_cptr *global_time)
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

    if(clock.skew)
    {
        Time skew_diff = *local_time;
        time_sub(&skew_diff, &clock.last_sync);
        time_add_i32(local_time, -100 * skew_diff.low / clock.skew);
        skew_diff.low = 0;
        time_sub(local_time, &skew_diff);
    }

    DEBUG(  "[" TIME_FMT "] local_time=" TIME_FMT " global_time=" TIME_FMT
            "\r\n", TIME_FMT_DATA(*local_time), TIME_FMT_DATA(*local_time),
            TIME_FMT_DATA(*global_time));
}

void add_sync_point(Time_cptr *local_time, Time_cptr *global_time,
                    const uint16_t seq_id, const uint16_t root_macaddr)
{
    // Check if same or equal already exist
    for(uint8_t s = 0; s < SETTINGS_SYNCHRONIZATION_POINTS; ++ s)
    {
        if(sync_point[s].state != STATE_VALID)
            continue;

        if(sync_point[s].root_macaddr != root_macaddr)
            continue;

        if((int16_t) (seq_id - sync_point[s].seq_id) >= 0)
            continue;

        // Found better already existing entry
        return;
    }

    // Add new entry
    for(uint8_t s = 0; s < SETTINGS_SYNCHRONIZATION_POINTS; ++ s)
    {
        if(sync_point[s].state == STATE_VALID)
            continue;

        sync_point[s].state         = STATE_VALID;
        sync_point[s].local_time    = *local_time;
        sync_point[s].global_time   = *global_time;
        sync_point[s].seq_id        = seq_id;
        sync_point[s].root_macaddr  = root_macaddr;
        ++ valid_sync_points;
        return;
    }

    uint16_t smallest_seq_id = seq_id;
    for(uint8_t s = 0; s < SETTINGS_SYNCHRONIZATION_POINTS; ++ s)
    {
        if(sync_point[s].root_macaddr == root_macaddr &&
            sync_point[s].seq_id < smallest_seq_id)
            smallest_seq_id = sync_point[s].seq_id;
    }

    for(uint8_t s = 0; s < SETTINGS_SYNCHRONIZATION_POINTS; ++ s)
    {
        if(sync_point[s].seq_id != smallest_seq_id)
            continue;

        sync_point[s].state         = STATE_VALID;
        sync_point[s].local_time    = *local_time;
        sync_point[s].global_time   = *global_time;
        sync_point[s].seq_id        = seq_id;
        sync_point[s].root_macaddr  = root_macaddr;
        ++ valid_sync_points;
        return;
    }

    WARNING("[" TIME_FMT "] Maximum number of valid sync points found!\r\n",
            TIME_FMT_DATA(*local_time));
}

void validate_sync_points(void)
{
    valid_sync_points = 0;
    for(uint8_t s = 0; s < SETTINGS_SYNCHRONIZATION_POINTS; ++ s)
    {
        if(sync_point[s].root_macaddr != root.macaddr)
        {
            sync_point[s].state = STATE_INVALID;
            continue;
        }

        else if(sync_point[s].state != STATE_VALID)
            sync_point[s].state = STATE_VALID;

        ++ valid_sync_points;
    }
}

void calculate_clock(Time_cptr *time)
{
    if(valid_sync_points < SETTINGS_SYNCHRONIZATION_POINTS / 2)
    {
#if !SYNCHRONIZATION_FAST_SYNC
        NOTICE( "[" TIME_FMT "] not enough synchronization points %u\r\n",
                TIME_FMT_DATA(*time), valid_sync_points);
#else
        NOTICE( "[" TIME_FMT "] not enough synchronization points %u"
                " for full sync\r\n", TIME_FMT_DATA(*time), valid_sync_points);

        int64_t new_offset = 0;
        uint16_t new_seq_id     = 0;
        uint64_t new_last_sync  = 0;
        uint8_t count = 0;
        for(uint8_t s = 0; s < SETTINGS_SYNCHRONIZATION_POINTS; ++ s)
        {
            if(sync_point[s].state != STATE_VALID)
                continue;

            uint64_t global_time = \
                ((1LL * sync_point[s].global_time.high) << 32) +
                sync_point[s].global_time.low;

            uint64_t local_time = \
                ((1LL * sync_point[s].local_time.high) << 32) +
                sync_point[s].local_time.low;

            int64_t offset = (int64_t) (global_time - local_time);

            new_offset += offset;
            if(new_seq_id < sync_point[s].seq_id)
                new_seq_id = sync_point[s].seq_id;

            new_last_sync += local_time;
            ++ count;
        }

        new_offset /= count;
        new_last_sync /= count;
        if(new_offset < 0)
        {
            clock.add = false;
            new_offset *= -1;
        }

        else
            clock.add = true;

        DEBUG(  "[" TIME_FMT "] previous clock_offset=%d:" TIME_FMT
                " clock_skew=%ld seq_id=%u last_sync=" TIME_FMT "\r\n",
                TIME_FMT_DATA(*time), clock.add, TIME_FMT_DATA(clock.offset),
                clock.skew, clock.seq_id, TIME_FMT_DATA(clock.last_sync));

        clock.offset.low = new_offset;
        clock.offset.high = new_offset >> 32;

        clock.last_sync.low = new_last_sync;
        clock.last_sync.high = new_last_sync >> 32;

        clock.seq_id = new_seq_id;

        NOTICE( "[" TIME_FMT "] new clock_offset=%d:" TIME_FMT
                " clock_skew=%ld seq_id=%u last_sync=" TIME_FMT "\r\n",
                TIME_FMT_DATA(*time), clock.add, TIME_FMT_DATA(clock.offset),
                clock.skew, clock.seq_id, TIME_FMT_DATA(clock.last_sync));
#endif // !SYNCHRONIZATION_FAST_SYNC
        return;
    }

    uint16_t leader = 0;
    uint8_t count = 0;
    for(uint8_t s = 0; s < SETTINGS_SYNCHRONIZATION_POINTS; ++ s)
    {
        if(sync_point[s].state != STATE_VALID)
            continue;

        if(leader != sync_point[s].global_time.high)
        {
            if(!count)
            {
                leader = sync_point[s].global_time.high;
                count = 1;
            }

            else
                -- count;
        }

        else
            ++ count;
    }

    count = 0;
    for(uint8_t s = 0; s < SETTINGS_SYNCHRONIZATION_POINTS; ++ s)
    {
        if(sync_point[s].state != STATE_VALID)
            continue;

        if(sync_point[s].global_time.high == leader)
            ++ count;
    }

    if(count < valid_sync_points / 2)
    {
        WARNING("[" TIME_FMT "] too big measurements differences (%u)\r\n",
                TIME_FMT_DATA(*time), count);
        return;
    }

    int64_t new_offset      = 0;
    int32_t new_skew        = clock.skew;
    uint16_t new_seq_id     = 0;
    uint64_t new_last_sync  = 0;

    int64_t local_sum           = 0;
    int64_t local_average_rest  = 0;
    int64_t offset_sum          = 0;
    int64_t offset_average_rest = 0;

    for(uint8_t s = 0; s < SETTINGS_SYNCHRONIZATION_POINTS; ++ s)
    {
        if(sync_point[s].state != STATE_VALID)
            continue;

        if(sync_point[s].global_time.high != leader)
            continue;

        uint64_t global_time = ((1LL * sync_point[s].global_time.high) << 32) +
                                sync_point[s].global_time.low;

        uint64_t local_time = ((1LL * sync_point[s].local_time.high) << 32) +
                                sync_point[s].local_time.low;

        new_last_sync = local_time;
        new_offset = (int64_t) global_time - local_time;
        break;
    }

    for(int s = 0; s < SETTINGS_SYNCHRONIZATION_POINTS; ++ s)
    {
        if(sync_point[s].state != STATE_VALID)
            continue;

        if(sync_point[s].global_time.high != leader)
            continue;

        uint64_t global_time = ((1LL * sync_point[s].global_time.high) << 32) +
                                sync_point[s].global_time.low;

        uint64_t local_time = ((1LL * sync_point[s].local_time.high) << 32) +
                                sync_point[s].local_time.low;

        int64_t offset = (int64_t) (global_time - local_time);

        local_sum += (int64_t) (local_time - new_last_sync) / count;
        local_average_rest += (local_time - new_last_sync) % count;
        offset_sum += (int64_t) (offset - new_offset) / count;
        offset_average_rest += (offset - new_offset) % count;
        if(sync_point[s].seq_id > new_seq_id)
            new_seq_id = sync_point[s].seq_id;
    }

    new_last_sync += local_sum + local_average_rest / count;
    new_offset += offset_sum + offset_average_rest / count;

    local_sum = 0;
    offset_sum = 0;

    for(int s = 0; s < SETTINGS_SYNCHRONIZATION_POINTS; ++ s)
    {
        if(sync_point[s].state != STATE_VALID)
            continue;

        if(sync_point[s].global_time.high != leader)
            continue;

        uint64_t global_time = ((1LL * sync_point[s].global_time.high) << 32) +
                                sync_point[s].global_time.low;

        uint64_t local_time = ((1LL * sync_point[s].local_time.high) << 32) +
                                sync_point[s].local_time.low;

        int64_t offset = (int64_t) (global_time - local_time);

        int32_t a = local_time - new_last_sync;
        int32_t b = offset - new_offset;

        local_sum += (int64_t) a * a;
        offset_sum += (int64_t) a * b;
    }

    if(local_sum != 0 && offset_sum != 0)
        new_skew = 100 * local_sum / offset_sum;

    DEBUG(  "[" TIME_FMT "] previous clock_offset=%d:" TIME_FMT
            " clock_skew=%ld seq_id=%u last_sync=" TIME_FMT "\r\n",
            TIME_FMT_DATA(*time), clock.add, TIME_FMT_DATA(clock.offset),
            clock.skew, clock.seq_id, TIME_FMT_DATA(clock.last_sync));

    if(new_offset < 0)
    {
        clock.add = false;
        new_offset *= -1;
    }

    else
        clock.add = true;

    clock.offset.low = new_offset;
    clock.offset.high = new_offset >> 32;

    clock.skew = new_skew;
    clock.last_sync.low = new_last_sync;
    clock.last_sync.high = new_last_sync >> 32;

    clock.seq_id = new_seq_id;

    NOTICE( "[" TIME_FMT "] new clock_offset=%d:" TIME_FMT
            " clock_skew=%ld seq_id=%u last_sync=" TIME_FMT "\r\n",
            TIME_FMT_DATA(*time), clock.add, TIME_FMT_DATA(clock.offset),
            clock.skew, clock.seq_id, TIME_FMT_DATA(clock.last_sync));
}

#endif // __MESSAGE_SYNCHRONIZATION_H__
