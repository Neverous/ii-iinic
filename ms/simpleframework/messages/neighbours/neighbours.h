#ifndef __MESSAGE_NEIGHBOURS_H__
#define __MESSAGE_NEIGHBOURS_H__

#include "struct.h"

uint8_t neighbours_ttl;

void on_init_MessageNeighbours( __unused__ Time_cptr time,
                                __unused__ const uint8_t options)
{
}

void on_frame_start_MessageNeighbours(  __unused__ Time_cptr frame_start,
                                        __unused__ Time *frame_deadline,
                                        __unused__ const uint8_t options)
{
}

void on_slot_start_MessageNeighbours(   __unused__ Time_cptr slot_start,
                                        __unused__ Time *slot_end,
                                        __unused__ const uint8_t options)
{
    if( iinic_read_button() &&
        neighbours_ttl < SETTINGS_NEIGHBOURS_SHOW_TIME / 2)
    {
        neighbours_ttl = SETTINGS_NEIGHBOURS_SHOW_TIME;
        iinic_led_on(IINIC_LED_RED);
        put_MessageNeighbours(slot_start, 0);
    }
}

void on_slot_end_MessageNeighbours( __unused__ Time_cptr slot_end,
                                    __unused__ const uint8_t options)
{
}

void on_frame_end_MessageNeighbours(__unused__ Time_cptr frame_end,
                                    __unused__ const uint8_t options)
{
    if(!neighbours_ttl)
        return;

    -- neighbours_ttl;
    if(!neighbours_ttl)
        iinic_led_off(IINIC_LED_RED);
}

void handle_MessageNeighbours(  Time_cptr time, const uint16_t rssi,
                                __unused__ MessageNeighbours_cptr msg,
                                const uint8_t options)
{
    DEBUG(  "[" TIME_FMT "] Got neighbours message options=0x%02x rssi=%u"
            " macaddr=0x%04x\r\n", TIME_FMT_DATA(*time), options, rssi,
            msg->macaddr);

    if(!neighbours_ttl)
        iinic_led_on(IINIC_LED_RED);

    neighbours_ttl = SETTINGS_NEIGHBOURS_SHOW_TIME;
}

uint8_t *write_MessageNeighbours(   __unused__ Time_cptr time,
                                    uint8_t *buffer_start,
                                    uint8_t_cptr buffer_end,
                                    __unused__ uint8_t *ctx)
{
    if(buffer_start + sizeof(MessageNeighbours) > buffer_end)
        return NULL;

    MessageNeighbours *msg  = (MessageNeighbours *) buffer_start;
    msg->base.kind          = KIND_NEIGHBOURS;
    msg->macaddr            = iinic_mac;
    return (uint8_t *) (msg + 1);
}

#endif // __MESSAGE_NEIGHBOURS_H__
