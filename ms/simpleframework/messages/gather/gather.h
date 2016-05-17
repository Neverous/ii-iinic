#ifndef __MESSAGE_GATHER_H__
#define __MESSAGE_GATHER_H__

#include "struct.h"
#include "messages/discovery/neighbours.h"

struct
{
    bool button;
    bool got[SETTINGS_MAX_SENSORS];
    uint8_t ttl;
    BackoffStats data;
} gather;

void on_init_MessageGather( __unused__ Time_cptr *time,
                            __unused__ const uint8_t options)
{
    gather.ttl = 0;
    gather.button = 0;
}

void on_init_MessageGatherRequest(  __unused__ Time_cptr *time,
                                    __unused__ const uint8_t options)
{
}

void on_frame_start_MessageGather(  Time_cptr *frame_start,
                                    __unused__ Time *frame_deadline,
                                    const uint8_t options)
{
    if(iinic_mac != root.macaddr)
        return;

    if(!(options & TDMA_EVENT))
        return;

    bool next = iinic_read_button();
    if(gather.button && !next)
    {
        NOTICE( "[" TIME_FMT "] Initializing gathering\r\n",
                TIME_FMT_DATA(*frame_start));

        for(uint8_t s = 0; s < SETTINGS_MAX_SENSORS; ++ s)
        {
            gather.got[s] = false;
            gather.data.sent[s] = 0;
            gather.data.received[s] = 0;
            for(uint8_t t = 0; t < SETTINGS_BACKOFF_TRIES_LIMIT; ++ t)
                gather.data.ack[s][t] = 0;
        }

        gather.ttl = SETTINGS_GATHER_PERIOD;
    }

    gather.button = next;
}

void on_frame_start_MessageGatherRequest(   __unused__ Time_cptr *frame_start,
                                            __unused__ Time *frame_deadline,
                                            __unused__ const uint8_t options)
{
    if(gather.ttl)
        iinic_led_toggle(IINIC_LED_RED);
}

void on_slot_start_MessageGather(   __unused__ Time_cptr *slot_start,
                                    __unused__ Time *slot_end,
                                    const uint8_t options)
{
    if(!(options & TDMA_EVENT))
        return;

    if(iinic_mac == root.macaddr)
        return;

    if(gather.ttl && (gather.ttl % SETTINGS_GATHER_MESSAGE_TTL) == 0)
        put_MessageGather(slot_start, 0);

}

void on_slot_start_MessageGatherRequest(__unused__ Time_cptr *slot_start,
                                        __unused__ Time *slot_end,
                                        const uint8_t options)
{
    if(!(options & TDMA_EVENT))
        return;

    if(iinic_mac != root.macaddr)
        return;

    if(gather.ttl && (gather.ttl % SETTINGS_GATHER_REQUEST_TTL) == 0)
    {
        uint8_t ttl = SETTINGS_GATHER_REQUEST_TTL;
        put_MessageGatherRequest(slot_start, &ttl);
    }
}

void on_slot_end_MessageGather( __unused__ Time_cptr *slot_end,
                                __unused__ const uint8_t options)
{
}

void on_slot_end_MessageGatherRequest(  __unused__ Time_cptr *slot_end,
                                        __unused__ const uint8_t options)
{
}

void on_frame_end_MessageGather(__unused__ Time_cptr *frame_end,
                                __unused__ const uint8_t options)
{
}

void on_frame_end_MessageGatherRequest( Time_cptr *frame_end,
                                        __unused__ const uint8_t options)
{
    if(!gather.ttl)
        return;

    if(gather.ttl == 1)
    {
        iinic_led_off(IINIC_LED_RED);
        show_backoff_stats(frame_end, &gather.data);
    }

    -- gather.ttl;
}

void handle_MessageGather(  Time_cptr *time,
                            const uint16_t rssi,
                            MessageGather_cptr *msg,
                            const uint8_t options)
{
    uint16_t size = message_get_size(&msg->base);
    NOTICE( "[" TIME_FMT "] Got gather message options=0x%02x rssi=%u "
            "ttl=%d macaddr=0x%04x size=%d\r\n", TIME_FMT_DATA(*time),
            options, rssi, msg->ttl, msg->macaddr, size);

    if(!gather.ttl)
        return;

    if(iinic_mac == msg->macaddr)
        return;

    if(iinic_mac != root.macaddr)
    {
        // Forward
        if(msg->ttl == 1)
            return;

        MessageGather *wr = (MessageGather *) msg;
        -- wr->ttl;
        put_message((uint8_t_cptr *) msg, size);
        return;
    }

    uint8_t dest_sensor_id = get_sensor_id(msg->macaddr);
    if(dest_sensor_id >= SETTINGS_MAX_SENSORS)
        return;

    if(gather.got[dest_sensor_id])
        return;

    gather.got[dest_sensor_id] = true;
    uint8_t *msg_end = (uint8_t *) msg + size;

    const IndexedData *stats = msg->stats;
    while((uint8_t *) stats < msg_end)
    {
        switch(stats->index_high)
        {
            case 0:
                gather.data.sent[stats->index_low] += stats->value;
                break;

            case 1:
                gather.data.received[stats->index_low] += stats->value;
                break;

            default:
                gather.data.ack[stats->index_low][stats->index_high - 2] +=
                    stats->value;
                break;
        }

        ++ stats;
    }

    show_backoff_stats(time, &gather.data);
}

void handle_MessageGatherRequest(   Time_cptr *time,
                                    const uint16_t rssi,
                                    MessageGatherRequest_cptr *msg,
                                    const uint8_t options)
{
    NOTICE( "[" TIME_FMT "] Got gather_request message options=0x%02x rssi=%u "
            "ttl=%d\r\n", TIME_FMT_DATA(*time), options, rssi, msg->ttl);

    if(iinic_mac == root.macaddr)
        return;

    if(!gather.ttl)
        gather.ttl = SETTINGS_GATHER_CYCLE;

    if(msg->ttl == 1)
        return;

    // Forward
    uint8_t ttl = msg->ttl - 1;
    put_MessageGatherRequest(time, &ttl);
}

uint8_t *write_MessageGather(   __unused__ Time_cptr *time,
                                uint8_t *buffer_start,
                                uint8_t_cptr *buffer_end,
                                __unused__ uint8_t *ctx)
{
    if(buffer_start + sizeof(MessageGather) > buffer_end)
        return 0;

    MessageGather *msg = (MessageGather *) buffer_start;
    message_set_kind(&msg->base, KIND_GATHER);
    msg->ttl        = SETTINGS_GATHER_MESSAGE_TTL;
    msg->macaddr    = iinic_mac;

    IndexedData *stats = msg->stats;
    for(uint8_t s = 0;  s < SETTINGS_MAX_SENSORS &&
                        (uint8_t *) (stats + 1) <= buffer_end; ++ s)
        if(backoff_stats.sent[s])
        {
            stats->index_high   = 0;
            stats->index_low    = s;
            stats->value        = backoff_stats.sent[s];
            ++ stats;
        }

    for(uint8_t s = 0;  s < SETTINGS_MAX_SENSORS &&
                        (uint8_t *) (stats + 1) <= buffer_end; ++ s)
        if(backoff_stats.received[s])
        {
            stats->index_high   = 1;
            stats->index_low    = s;
            stats->value        = backoff_stats.received[s];
            ++ stats;
        }

    for(uint8_t s = 0;  s < SETTINGS_MAX_SENSORS &&
                        (uint8_t *) (stats + 1) <= buffer_end; ++ s)
        for(uint8_t t = 0;  t < SETTINGS_BACKOFF_TRIES_LIMIT &&
                            (uint8_t *) (stats + 1) <= buffer_end; ++ t)
            if(backoff_stats.ack[s][t])
            {
                stats->index_high   = 2 + t;
                stats->index_low    = s;
                stats->value        = backoff_stats.ack[s][t];
                ++ stats;
            }

    message_set_size(&msg->base, (uint8_t *) stats - buffer_start);
    return (uint8_t *) stats;
}

uint8_t *write_MessageGatherRequest(__unused__ Time_cptr *time,
                                    uint8_t *buffer_start,
                                    uint8_t_cptr *buffer_end,
                                    uint8_t *ttl)
{
    if(buffer_start + sizeof(MessageGatherRequest) > buffer_end)
        return 0;

    MessageGatherRequest *msg = (MessageGatherRequest *) buffer_start;
    msg->base.kind  = KIND_GATHER_REQUEST;
    msg->ttl        = *ttl;
    return (uint8_t *) (msg + 1);
}

#endif // __MESSAGE_GATHER_H__
