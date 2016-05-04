#ifndef __MESSAGE_BACKOFF_H__
#define __MESSAGE_BACKOFF_H__

#include "struct.h"

void on_init_MessageBackoff(__unused__ Time_cptr *time,
                            __unused__ const uint8_t options)
{
}

void on_frame_start_MessageBackoff( __unused__ Time_cptr *frame_start,
                                    __unused__ Time *frame_deadline,
                                    __unused__ const uint8_t options)
{
}

void on_slot_start_MessageBackoff(  __unused__ Time_cptr *slot_start,
                                    __unused__ Time *slot_end,
                                    __unused__ const uint8_t options)
{
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
}

uint8_t *write_MessageBackoff(  __unused__ Time_cptr *time,
                                uint8_t *buffer_start,
                                __unused__ const uint8_t const *buffer_end,
                                __unused__ uint8_t *ctx)
{
    return buffer_start;
}

#endif // __MESSAGE_BACKOFF_H__
