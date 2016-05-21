#ifndef __MESSAGE_BACKOFF_H__
#define __MESSAGE_BACKOFF_H__

#include <stdlib.h>

#include "messages/discovery/neighbours.h"
#include "sensors_config.h"
#include "struct.h"
#include "stats.h"

struct
{
    Time        next_try;
    uint16_t    dest_macaddr;
    uint8_t     ppb;
    uint8_t     tries_count;
    uint8_t     last_received[SETTINGS_MAX_SENSORS];
    uint8_t     seq_id;
    uint8_t     count;
} backoff;

BackoffStats backoff_stats;

uint16_t get_random_neighbour_macaddr(void);

void on_init_MessageBackoff(Time_cptr time,
                            const uint8_t options)
{
    if(options == MAIN_EVENT)
    {
        backoff.ppb = sensor_config_get_backoff_ppb(sensor_id);
    }

    if(options != TDMA_EVENT)
        return;


    DEBUG(  "[" TIME_FMT "] backoff_ppb=%d\n",
            TIME_FMT_DATA(*time), backoff.ppb);
}

void on_init_MessageBackoffAck( __unused__ Time_cptr time,
                                __unused__ const uint8_t options)
{
}

void on_frame_start_MessageBackoff( __unused__ Time_cptr frame_start,
                                    __unused__ Time *frame_deadline,
                                    __unused__ const uint8_t options)
{
}

void on_frame_start_MessageBackoffAck(  __unused__ Time_cptr frame_start,
                                        __unused__ Time *frame_deadline,
                                        __unused__ const uint8_t options)
{
}

void on_slot_start_MessageBackoff(  Time_cptr slot_start,
                                    __unused__ Time *slot_end,
                                    const uint8_t options)
{
    if(options != TDMA_EVENT)
        return;

    if(backoff.tries_count)
    {
        if(backoff.tries_count > SETTINGS_BACKOFF_TRIES_LIMIT)
        {
            backoff.tries_count = 0;
            return;
        }

        if(time_cmp(slot_start, &backoff.next_try) < 0)
            return;
    }

    else if(random() % 256 < backoff.ppb)
        backoff.dest_macaddr = get_random_neighbour_macaddr();

    else
        return;

    uint16_t dest_sensor_id = get_sensor_id(backoff.dest_macaddr);
    if(dest_sensor_id > SETTINGS_MAX_SENSORS - 1)
        return;

    ++ backoff_stats.sent[dest_sensor_id];
    ++ backoff.tries_count;
    backoff.next_try = *slot_start;
    time_add_i32(   &backoff.next_try,
                    (1 << backoff.tries_count) * SETTINGS_TDMA_FRAME_TIME);
    put_MessageBackoff(slot_start, (uint8_t *) &backoff.dest_macaddr);
}

void on_slot_start_MessageBackoffAck(   __unused__ Time_cptr slot_start,
                                        __unused__ Time *slot_end,
                                        __unused__ const uint8_t options)
{
}

void on_slot_end_MessageBackoff(__unused__ Time_cptr slot_end,
                                __unused__ const uint8_t options)
{
}

void on_slot_end_MessageBackoffAck( __unused__ Time_cptr slot_end,
                                    __unused__ const uint8_t options)
{
}

void on_frame_end_MessageBackoff(   __unused__ Time_cptr frame_end,
                                    __unused__ const uint8_t options)
{
}

void on_frame_end_MessageBackoffAck(Time_cptr frame_end,
                                    __unused__ const uint8_t options)
{
    if((backoff.count ++) % 64 == 0)
        show_backoff_stats(frame_end, &backoff_stats);
}

void handle_MessageBackoff( Time_cptr time,
                            const uint16_t rssi,
                            MessageBackoff_cptr msg,
                            const uint8_t options)
{
    DEBUG(  "[" TIME_FMT "] Got backoff message options=0x%02x rssi=%u "
            "src_macaddr=0x%04x dest_macaddr=0x%04x\r\n", TIME_FMT_DATA(*time),
            options, rssi, msg->src_macaddr, msg->dest_macaddr);

    if(msg->dest_macaddr != iinic_mac)
        return;

    uint16_t src_sensor_id = get_sensor_id(msg->src_macaddr);
    if(src_sensor_id > SETTINGS_MAX_SENSORS - 1)
        return;

    if((int8_t) (msg->seq_id - backoff.last_received[src_sensor_id]) <= 0)
        return;

    ++ backoff_stats.received[src_sensor_id];
    backoff.last_received[src_sensor_id] = msg->seq_id;
    put_MessageBackoffAck(time, (uint8_t *) &msg->src_macaddr);
}

void handle_MessageBackoffAck(  Time_cptr time,
                                const uint16_t rssi,
                                MessageBackoffAck_cptr msg,
                                const uint8_t options)
{
    DEBUG(  "[" TIME_FMT "] Got backoffack message options=0x%02x rssi=%u "
            "src_macaddr=0x%04x dest_macaddr=0x%04x\r\n", TIME_FMT_DATA(*time),
            options, rssi, msg->src_macaddr, msg->dest_macaddr);

    if(!backoff.tries_count)
        return;

    if(backoff.tries_count > SETTINGS_BACKOFF_TRIES_LIMIT)
        return;

    if( msg->dest_macaddr != iinic_mac ||
        msg->src_macaddr != backoff.dest_macaddr)
        return;

    ++ backoff_stats.ack[get_sensor_id(backoff.dest_macaddr)][
        backoff.tries_count - 1];

    backoff.tries_count = 0;
}

uint8_t *write_MessageBackoff(  __unused__ Time_cptr time,
                                uint8_t *buffer_start,
                                uint8_t_cptr buffer_end,
                                uint8_t *macaddr)
{
    if(buffer_start + sizeof(MessageBackoff) > buffer_end)
        return NULL;

    MessageBackoff *msg = (MessageBackoff *) buffer_start;
    msg->base.kind      = KIND_BACKOFF;
    msg->seq_id         = ++ backoff.seq_id;
    msg->src_macaddr    = iinic_mac;
    msg->dest_macaddr   = *(uint16_t *) macaddr;
    return (uint8_t *) (msg + 1);
}

uint8_t *write_MessageBackoffAck(   __unused__ Time_cptr time,
                                    uint8_t *buffer_start,
                                    uint8_t_cptr buffer_end,
                                    uint8_t *macaddr)
{
    if(buffer_start + sizeof(MessageBackoffAck) > buffer_end)
        return NULL;

    MessageBackoffAck *msg = (MessageBackoffAck *) buffer_start;
    msg->base.kind      = KIND_BACKOFFACK;
    msg->src_macaddr    = iinic_mac;
    msg->dest_macaddr   = *(uint16_t *) macaddr;
    return (uint8_t *) (msg + 1);
}

uint16_t get_random_neighbour_macaddr(void)
{
    if(!get_neighbours_count())
        return -1;

    uint8_t _neighbour = random() % get_neighbours_count();
    for(uint8_t n = 0; n < SETTINGS_MAX_SENSORS; ++ n)
    {
        if(!neighbour[n].ttl)
            continue;

        if(!_neighbour)
            return neighbour[n].macaddr;

        -- _neighbour;
    }

    return -1;
}

void show_backoff_stats(Time_cptr time, BackoffStats *stats)
{
    NOTICE("[" TIME_FMT "] backoff stats", TIME_FMT_DATA(*time));
    NOTICE("\r\n    sent: ");
    for(uint8_t s = 0; s < SETTINGS_MAX_SENSORS; ++ s)
        NOTICE("%d ", stats->sent[s]);

    NOTICE("\r\n    ack:");
    for(uint8_t s = 0; s < SETTINGS_MAX_SENSORS; ++ s)
    {
        NOTICE("\r\n        ");
        for(uint8_t t = 0; t < SETTINGS_BACKOFF_TRIES_LIMIT; ++ t)
            NOTICE("%d ", stats->ack[s][t]);
    }

    NOTICE("\r\n    received: ");
    for(uint8_t s = 0; s < SETTINGS_MAX_SENSORS; ++ s)
        NOTICE("%d ", stats->received[s]);

    NOTICE("\r\n");
}

#endif // __MESSAGE_BACKOFF_H__
