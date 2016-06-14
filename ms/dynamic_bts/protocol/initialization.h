#ifndef __PROTOCOL_INITIALIZATION_H__
#define __PROTOCOL_INITIALIZATION_H__

#include "blinker.h"
#include "messages.h"
#include "usart.h"

void initialization_handle_messages(
    Time_cptr time, const uint16_t rssi,
    uint8_t *buffer_ptr, uint8_t_cptr buffer_end);

inline
static
void initialization_loop(void)
{
    iinic_idle();
    iinic_set_buffer(rxbuffer, SETTINGS_RXBUFFER_SIZE);
    iinic_rx();

    for(uint8_t r = 0; r < SETTINGS_INITIALIZATION_FRAMES; ++ r)
    {
        Time local_time; time_get_now(&local_time);
        NOTICE(TIME_FMT "|M|INIT(%u)\r\n", TIME_FMT_DATA(local_time), r);
        Time global_time; time_local_to_global(&global_time, &local_time);
        blink(&global_time);
        time_align(&global_time,    SETTINGS_CONTROL_FRAME_TIME
                                    + SETTINGS_DATA_FRAME_TIME);

        Time frame_deadline; time_global_to_local(  &frame_deadline,
                                                    &global_time);

        uint8_t signal;
        while((signal = timed_poll( IINIC_RX_COMPLETE | IINIC_SIGNAL,
                                    &frame_deadline)))
        {
            if(signal & IINIC_RX_COMPLETE)
            {
                initialization_handle_messages(
                    (Time_cptr) &iinic_rx_timing,
                    iinic_rx_rssi,
                    rxbuffer, iinic_buffer_ptr);

                iinic_rx();
            }

            if(signal & IINIC_SIGNAL)
                handle_usart();
        }

        validate_synchronization();
    }
}

inline
void initialization_handle_messages(
    Time_cptr time, const uint16_t rssi,
    uint8_t *buffer_ptr, uint8_t_cptr buffer_end)
{
    DEBUG(  TIME_FMT "|R|+%ub\r\n",
            TIME_FMT_DATA(*time), buffer_end - buffer_ptr);

    uint8_t count = 0;
    while(buffer_ptr < buffer_end)
    {
        Message_cptr msg = (Message_cptr) buffer_ptr;
        switch(validate_message(msg, &buffer_ptr, buffer_end))
        {
            case KIND_SYNCHRONIZATION:
                handle_synchronization(
                    time, (MessageSynchronization_cptr) msg, rssi);
                ++ count;
                break;

            case KIND_EOF:
                ++ buffer_ptr;
                break;
        }
    }

    DEBUG(TIME_FMT "|R|+%um\r\n", TIME_FMT_DATA(*time), count);
}

#endif // __PROTOCOL_INITIALIZATION_H__
