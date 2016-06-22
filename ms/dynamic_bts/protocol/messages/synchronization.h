#ifndef __PROTOCOL_MESSAGES_SYNCHRONIZATION_H__
#define __PROTOCOL_MESSAGES_SYNCHRONIZATION_H__

#include <limits.h>

#include "common.h"

//
// MessageSynchronization
//  Wiadomość służąca do wykrywania sąsiadów, elekcji korzenia
//  oraz synchronizacji czasu
//    rozmiar: 12 bajtów
//
//  [3]     kind            - typ wiadomości = 0x1
//  [5]     unused          - nieużywane
//  [16]    macaddr         - adres MAC nadawcy
//  [16]    root_macaddr    - aktualny korzeń nadawcy
//  [8]     seq_id          - numer rundy synchronizacji
//  [48]    global_time     - globalny czas według nadawcy
//

#define KIND_SYNCHRONIZATION 0x1

typedef struct message_synchronization
{
    uint8_t kind    : 3;
    uint8_t unused  : 5;

    uint16_t    macaddr;
#ifndef STATIC_ROOT
    uint16_t    root_macaddr;
#endif
    uint8_t     seq_id;
    Time        global_time;
} MessageSynchronization;

typedef const MessageSynchronization * const MessageSynchronization_cptr;

uint8_t message_synchronization_get_size(
    __unused__ MessageSynchronization_cptr msg)
{
    return sizeof(MessageSynchronization);
}


#ifdef __AVR__
////////////////////////////////////////////////////////////////////////////////

struct SynchronizationPoint
{
    Time        local_time;
    Time        global_time;
    uint8_t     ttl;
};

struct SynchronizationData
{
    struct
    {
        bool    add;
        Time    offset;
        float   skew;
        Time    last_sync;
        uint8_t seq_id;
        uint8_t prev_seq_id;
    } clock;

    struct
    {
        bool    valid;
        bool    trigger;
        uint8_t counter;
    } timer;

#ifndef STATIC_ROOT
    struct
    {
        uint16_t    macaddr;
        uint16_t    prev_macaddr;
        uint8_t     ttl;
    } root;
#endif

    struct SynchronizationPoint sync_point[SETTINGS_SYNCHRONIZATION_POINTS];
};

extern struct SynchronizationData synchronization;


////////////////////////////////////////////////////////////////////////////////

extern void update_neighbour(   Time_cptr time,
                                const uint16_t macaddr,
                                const uint8_t rssi);

void handle_synchronization(Time_cptr time, MessageSynchronization_cptr msg,
                            const uint8_t rssi);

void put_synchronization_message(void);

void time_get_global_now(Time *global_time);
void time_local_to_global(Time *global_time, Time_cptr local_time);
void time_global_to_local(Time *local_time, Time_cptr global_time);

void update_synchronization_point(
#ifndef STATIC_ROOT
        const uint16_t root_macaddr,
#endif
        const uint8_t seq_id,
        Time_cptr local_time,
        Time_cptr global_time);

void validate_synchronization(void);
void recalculate_clock(Time_cptr time);


////////////////////////////////////////////////////////////////////////////////

void handle_synchronization(Time_cptr time, MessageSynchronization_cptr msg,
                            const uint8_t rssi)
{
    DEBUG(  TIME_FMT "|R|+SYNC(%u,0x%04x,0x%04x,%u," TIME_FMT ")\r\n",
            TIME_FMT_DATA(*time), rssi, msg->macaddr,
#ifndef STATIC_ROOT
            msg->root_macaddr,
#else
            STATIC_ROOT,
#endif
            msg->seq_id, TIME_FMT_DATA(msg->global_time));

    update_neighbour(time, msg->macaddr, rssi);
    update_synchronization_point(
#ifndef STATIC_ROOT
            msg->root_macaddr,
#endif
            msg->seq_id,
            time,
            &msg->global_time);
}

void put_synchronization_message(void)
{
    MessageSynchronization *msg =
        (MessageSynchronization *) control_txbuffer_get(
                                            sizeof(MessageSynchronization));

    if(!msg)
        return;

    synchronization.timer.trigger = false;

    msg->kind           = KIND_SYNCHRONIZATION;
    msg->macaddr        = device_macaddr;
#ifndef STATIC_ROOT
    msg->root_macaddr   = synchronization.root.macaddr;
#endif
    msg->seq_id         = synchronization.clock.seq_id;
    Time local_time; time_get_now(&local_time);
    time_local_to_global(&msg->global_time, &local_time);

    control_txbuffer_commit(sizeof(MessageSynchronization));
#ifndef STATIC_ROOT
    if(synchronization.root.macaddr == device_macaddr)
#else
    if(STATIC_ROOT == device_macaddr)
#endif
        ++ synchronization.clock.seq_id;

    DEBUG(  TIME_FMT "|R|-SYNC(-1,0x%04x,0x%04x,%u," TIME_FMT ")\r\n",
            TIME_FMT_DATA(local_time), msg->macaddr,
#ifndef STATIC_ROOT
            msg->root_macaddr,
#else
            STATIC_ROOT,
#endif
            msg->seq_id, TIME_FMT_DATA(msg->global_time));
}

void time_get_global_now(Time *global_time)
{
    Time local_time; time_get_now(&local_time);
    time_local_to_global(global_time, &local_time);
}

void time_local_to_global(Time *global_time, Time_cptr local_time)
{
    if(!local_time)
        return;

    *global_time = *local_time;
    if(synchronization.clock.add)
        time_add(global_time, &synchronization.clock.offset);

    else
        time_sub(global_time, &synchronization.clock.offset);

    Time skew_diff = *local_time;
    time_sub(&skew_diff, &synchronization.clock.last_sync);
    time_add_i32(global_time, synchronization.clock.skew * skew_diff.low);
    time_add_i32(   global_time,
                    (synchronization.clock.skew * UINT_MAX) * skew_diff.high);
}

void time_global_to_local(Time *local_time, Time_cptr global_time)
{
    if(!global_time)
        return;

    *local_time = *global_time;
    if(synchronization.clock.add)
        time_sub(local_time, &synchronization.clock.offset);

    else
        time_add(local_time, &synchronization.clock.offset);

    Time skew_diff = *local_time;
    time_sub(&skew_diff, &synchronization.clock.last_sync);
    time_add_i32(local_time, -synchronization.clock.skew * skew_diff.low);
    time_add_i32(   local_time,
                    -(synchronization.clock.skew * UINT_MAX) * skew_diff.high);
}

void update_synchronization_point(
#ifndef STATIC_ROOT
        const uint16_t root_macaddr,
#endif
        const uint8_t seq_id,
        Time_cptr local_time,
        Time_cptr global_time)
{
#ifndef STATIC_ROOT
    if(root_macaddr < synchronization.root.macaddr)
    {
        // Poprawka dla reelekcji
        if(     root_macaddr == synchronization.root.prev_macaddr
            &&  (int8_t) (seq_id - synchronization.clock.prev_seq_id) <= 0)
            return;

        DEBUG(  TIME_FMT "| |ROOT=0x%04x\r\n",
                TIME_FMT_DATA(*local_time), root_macaddr);

        synchronization.root.macaddr = root_macaddr;
    }

    else if(    root_macaddr > synchronization.root.macaddr
            ||  (int8_t) (seq_id - synchronization.clock.seq_id) <= 0)
        return;
#else
    if((int8_t) (seq_id - synchronization.clock.seq_id) <= 0)
        return;
#endif

    synchronization.clock.seq_id = seq_id;

#ifndef STATIC_ROOT
    if(synchronization.root.macaddr < device_macaddr)
        synchronization.root.ttl = SETTINGS_ROOT_TTL;
#endif

    // Podmień najstarszy wpis
    struct SynchronizationPoint *sync_point = synchronization.sync_point;
    uint8_t smallest_ttl = SETTINGS_SYNCHRONIZATION_TTL;
    for(uint8_t s = 0; s < SETTINGS_SYNCHRONIZATION_POINTS; ++ s)
        if(smallest_ttl > sync_point[s].ttl)
            smallest_ttl = sync_point[s].ttl;

    for(uint8_t s = 0; s < SETTINGS_SYNCHRONIZATION_POINTS; ++ s)
        if(sync_point[s].ttl == smallest_ttl)
        {
            sync_point[s].local_time    = *local_time;
            sync_point[s].global_time   = *global_time;
            sync_point[s].ttl           = SETTINGS_SYNCHRONIZATION_TTL;
            recalculate_clock(local_time);
            return;
        }
}

void validate_synchronization(void)
{
    for(uint8_t s = 0; s < SETTINGS_SYNCHRONIZATION_POINTS; ++ s)
        if(synchronization.sync_point[s].ttl)
            -- synchronization.sync_point[s].ttl;

#ifndef STATIC_ROOT
    if(synchronization.root.ttl)
        -- synchronization.root.ttl;

    else if(synchronization.root.macaddr != device_macaddr)
    {
        DEBUG(  TIME_FMT "| |ROOT=0x%04x\r\n",
                (uint16_t) 0, (uint32_t) 0, device_macaddr);
        synchronization.root.prev_macaddr   = synchronization.root.macaddr;
        synchronization.clock.prev_seq_id   = synchronization.clock.seq_id;
        synchronization.root.macaddr        = device_macaddr;
        synchronization.timer.trigger       = true;
    }
#endif

    ++ synchronization.timer.counter;
}

uint8_t _find_leader(uint16_t *leader)
{
    struct SynchronizationPoint *sync_point = synchronization.sync_point;

    uint8_t count = 0;
    for(uint8_t s = 0; s < SETTINGS_SYNCHRONIZATION_POINTS; ++ s)
    {
        if(!sync_point[s].ttl)
            continue;

        if(!count)
            *leader = sync_point[s].global_time.high;

        if(*leader == sync_point[s].global_time.high)
            ++ count;

        else
            -- count;
    }

    count = 0;
    for(uint8_t s = 0; s < SETTINGS_SYNCHRONIZATION_POINTS; ++ s)
    {
        if(!sync_point[s].ttl)
            continue;

        if(*leader == sync_point[s].global_time.high)
            ++ count;
    }

    return count;
}

void recalculate_clock_fast(Time_cptr time, uint16_t leader, uint8_t count)
{
    struct SynchronizationPoint *sync_point = synchronization.sync_point;

    int64_t     new_offset      = 0;
    uint64_t    new_last_sync   = 0;

    NOTICE(TIME_FMT "| |FAST_SYNC(%u)\r\n", TIME_FMT_DATA(*time), count);
    for(uint8_t s = 0; s < SETTINGS_SYNCHRONIZATION_POINTS; ++ s)
    {
        if(!sync_point[s].ttl || leader != sync_point[s].global_time.high)
            continue;

        uint64_t _global_time =
            (((uint64_t) sync_point[s].global_time.high) << 32) +
            sync_point[s].global_time.low;

        uint64_t _local_time =
            (((uint64_t) sync_point[s].local_time.high) << 32) +
            sync_point[s].local_time.low;

        int64_t offset = (int64_t) (_global_time - _local_time);

        new_offset      += offset;
        new_last_sync   += _local_time;
    }

    new_offset      /= count;
    new_last_sync   /= count;

    DEBUG(  TIME_FMT "| |OLD_CLOCK(%d:" TIME_FMT ",%d," TIME_FMT ")\r\n",
            TIME_FMT_DATA(*time), synchronization.clock.add,
            TIME_FMT_DATA(synchronization.clock.offset),
            synchronization.clock.skew,
            TIME_FMT_DATA(synchronization.clock.last_sync));

    if(new_offset < 0)
    {
        synchronization.clock.add = false;
        new_offset *= -1;
    }

    else
        synchronization.clock.add = true;

    synchronization.clock.offset.low        = new_offset;
    synchronization.clock.offset.high       = new_offset >> 32;
    synchronization.clock.skew              = 0;
    synchronization.clock.last_sync.low     = new_last_sync;
    synchronization.clock.last_sync.high    = new_last_sync >> 32;
    synchronization.timer.trigger           = true;
    synchronization.timer.valid             = true;

    DEBUG(  TIME_FMT "| |NEW_CLOCK(%d:" TIME_FMT ",%d," TIME_FMT ")\r\n",
            TIME_FMT_DATA(*time), synchronization.clock.add,
            TIME_FMT_DATA(synchronization.clock.offset),
            synchronization.clock.skew,
            TIME_FMT_DATA(synchronization.clock.last_sync));
}

void recalculate_clock(Time_cptr time)
{
    struct SynchronizationPoint *sync_point = synchronization.sync_point;

    uint8_t     s;
    uint16_t    leader;
    uint8_t     count = _find_leader(&leader);

    int64_t     new_offset      = 0;
    uint64_t    new_last_sync   = 0;
    if(count < SETTINGS_SYNCHRONIZATION_POINTS / 2)
    {
        recalculate_clock_fast(time, leader, count);
        return;
    }

    float new_skew = synchronization.clock.skew;

    int64_t local_sum           = 0;
    int64_t local_average_rest  = 0;
    int64_t offset_sum          = 0;
    int64_t offset_average_rest = 0;
    for(s = 0; s < SETTINGS_SYNCHRONIZATION_POINTS; ++ s)
    {
        if(!sync_point[s].ttl || leader != sync_point[s].global_time.high)
            continue;

        uint64_t _global_time =
            (((uint64_t) sync_point[s].global_time.high) << 32) +
            sync_point[s].global_time.low;

        uint64_t _local_time =
            (((uint64_t) sync_point[s].local_time.high) << 32) +
            sync_point[s].local_time.low;

        new_last_sync   = _local_time;
        new_offset      = (int64_t) (_global_time - _local_time);
        break;
    }

    for(; s < SETTINGS_SYNCHRONIZATION_POINTS; ++ s)
    {
        if(!sync_point[s].ttl || leader != sync_point[s].global_time.high)
            continue;

        uint64_t _global_time =
            (((uint64_t) sync_point[s].global_time.high) << 32) +
            sync_point[s].global_time.low;

        uint64_t _local_time =
            (((uint64_t) sync_point[s].local_time.high) << 32) +
            sync_point[s].local_time.low;

        int64_t offset = (int64_t) (_global_time - _local_time);

        local_sum           += (int64_t) (_local_time - new_last_sync) / count;
        local_average_rest  += (int64_t) (_local_time - new_last_sync) % count;
        offset_sum          += (offset - new_offset) / count;
        offset_average_rest += (offset - new_offset) % count;
    }

    new_last_sync   += local_sum + local_average_rest / count;
    new_offset      += offset_sum + offset_average_rest / count;

    local_sum = 0;
    offset_sum = 0;
    for(s = 0; s < SETTINGS_SYNCHRONIZATION_POINTS; ++ s)
    {
        if(!sync_point[s].ttl || leader != sync_point[s].global_time.high)
            continue;

        uint64_t _global_time =
            (((uint64_t) sync_point[s].global_time.high) << 32) +
            sync_point[s].global_time.low;

        uint64_t _local_time =
            (((uint64_t) sync_point[s].local_time.high) << 32) +
            sync_point[s].local_time.low;

        int64_t offset = (int64_t) (_global_time - _local_time);

        int32_t a = _local_time - new_last_sync;
        int32_t b = offset - new_offset;

        local_sum   += (int64_t) a * a;
        offset_sum  += (int64_t) a * b;
    }

    if(local_sum != 0)
        new_skew = (float) offset_sum / local_sum;

    DEBUG(  TIME_FMT "| |OLD_CLOCK(%d:" TIME_FMT ",%d," TIME_FMT ")\r\n",
            TIME_FMT_DATA(*time), synchronization.clock.add,
            TIME_FMT_DATA(synchronization.clock.offset),
            synchronization.clock.skew,
            TIME_FMT_DATA(synchronization.clock.last_sync));

    if(new_offset < 0)
    {
        synchronization.clock.add = false;
        new_offset *= -1;
    }

    else
        synchronization.clock.add = true;

    synchronization.clock.offset.low    = new_offset;
    synchronization.clock.offset.high   = new_offset >> 32;
    if(-1 > new_skew || new_skew > 1)
        synchronization.clock.skew      = 0;

    else
        synchronization.clock.skew      = new_skew;

    synchronization.clock.last_sync.low     = new_last_sync;
    synchronization.clock.last_sync.high    = new_last_sync >> 32;
    synchronization.timer.trigger           = true;
    synchronization.timer.valid             = true;

    DEBUG(  TIME_FMT "| |NEW_CLOCK(%d:" TIME_FMT ",%d," TIME_FMT ")\r\n",
            TIME_FMT_DATA(*time), synchronization.clock.add,
            TIME_FMT_DATA(synchronization.clock.offset),
            synchronization.clock.skew,
            TIME_FMT_DATA(synchronization.clock.last_sync));
}

#endif // __AVR__
#endif // __PROTOCOL_MESSAGES_SYNCHRONIZATION_H__
