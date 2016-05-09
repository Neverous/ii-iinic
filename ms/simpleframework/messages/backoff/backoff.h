#ifndef __MESSAGE_BACKOFF_H__
#define __MESSAGE_BACKOFF_H__

#include <stdlib.h>

#include "sensors_config.h"
#include "struct.h"

uint8_t backoff_ppb;

void on_init_MessageBackoff(Time_cptr *time,
                            const uint8_t options)
{
    if(options != TDMA_EVENT)
        return;

    backoff_ppb = sensor_config_get_backoff_ppb(sensor_id);

    DEBUG(  "[" TIME_FMT "] backoff_ppb=%d\n",
            TIME_FMT_DATA(*time), backoff_ppb);
}

void on_frame_start_MessageBackoff( __unused__ Time_cptr *frame_start,
                                    __unused__ Time *frame_deadline,
                                    __unused__ const uint8_t options)
{
}

void on_slot_start_MessageBackoff(  Time_cptr *slot_start,
                                    __unused__ Time *slot_end,
                                    const uint8_t options)
{
    if(options != TDMA_EVENT)
        return;

    if(random() % 256 > backoff_ppb)
        return;

    put_MessageBackoff(slot_start, 0);
}

void on_slot_end_MessageBackoff(__unused__ Time_cptr *slot_end,
                                __unused__ const uint8_t options)
{
}

void on_frame_end_MessageBackoff(   __unused__ Time_cptr *slot_end,
                                    __unused__ const uint8_t options)
{
}

void handle_MessageBackoff( __unused__ Time_cptr *time,
                            __unused__ const uint16_t rssi,
                            __unused__ MessageBackoff_cptr *msg,
                            __unused__ const uint8_t options)
{
    NOTICE( "[" TIME_FMT "] Got backoff message options=0x%02x rssi=%u "
            " macaddr=0x%04x\r\n", TIME_FMT_DATA(*time), options, rssi,
            msg->macaddr);
}

uint8_t *write_MessageBackoff(  __unused__ Time_cptr *time,
                                uint8_t *buffer_start,
                                const uint8_t const *buffer_end,
                                __unused__ uint8_t *ctx)
{
    if(buffer_start + sizeof(MessageBackoff) > buffer_end)
        return 0;

    MessageBackoff *msg = (MessageBackoff *) buffer_start;
    msg->base.kind      = KIND_BACKOFF;
    msg->macaddr        = iinic_mac;
    return (uint8_t *) (msg + 1);
}

#endif // __MESSAGE_BACKOFF_H__
