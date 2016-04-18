#ifndef __INITIALIZATION_H__
#define __INITIALIZATION_H__

#include <stdlib.h>
#include "config.h"
#include "protocol/handlers.h"

void initialization_listen_until(Time_cptr *now, Time_cptr *until);
void initialization_speak_until(Time_cptr *now, Time_cptr *until);
void initialization_loop(void);


void initialization_loop(void)
{
    iinic_idle();
    iinic_set_buffer(rxbuffer, SETTINGS_RXBUFFER_SIZE);
    iinic_rx();
    Time deadline; time_get_now(&deadline);
    NOTICE("\r\n[" TIME_FMT "] :: Initializing\r\n", TIME_FMT_DATA(deadline));
    time_add_i32(
        &deadline,
        SETTINGS_INITIALIZATION_TIME * SETTINGS_INITIALIZATION_FRAME_TIME);

    on_init(&deadline, INITIALIZATION_EVENT);

    for(uint8_t r = 1; time_cmp_now(&deadline) < 0; ++ r)
    {
        Time frame_start; time_get_now(&frame_start);
        Time frame_deadline = frame_start;
        time_align(&frame_deadline, SETTINGS_INITIALIZATION_FRAME_TIME);

        on_frame_start(&frame_start, &frame_deadline, INITIALIZATION_EVENT);

        Time slot_start = frame_deadline;
        NOTICE( "[" TIME_FMT "] :::: Frame %u\r\n",
                TIME_FMT_DATA(frame_deadline), r);

        time_add_i32(   &slot_start,
                        random() % SETTINGS_INITIALIZATION_FRAME_TIME);

        initialization_listen_until(&frame_start, &slot_start);

        on_slot_start(&slot_start, &frame_deadline, INITIALIZATION_EVENT);

        initialization_speak_until(&frame_start, &frame_deadline);

        on_slot_end(0, INITIALIZATION_EVENT);

        initialization_listen_until(&frame_start, &frame_deadline);

        on_frame_end(&frame_deadline, INITIALIZATION_EVENT);

        iinic_led_toggle(IINIC_LED_GREEN);
    }

    iinic_led_off(IINIC_LED_GREEN);
}

void initialization_listen_until(Time_cptr *now, Time_cptr *until)
{
    DEBUG(  "[" TIME_FMT "] listening until " TIME_FMT "\r\n",
            TIME_FMT_DATA(*now), TIME_FMT_DATA(*until));

    while(timed_poll(IINIC_RX_COMPLETE, until))
    {
        handle_messages((Time_cptr *) &iinic_rx_timing, iinic_rx_rssi,
                        rxbuffer, iinic_buffer_ptr, INITIALIZATION_EVENT);
        iinic_rx();
    }
}

void initialization_speak_until(Time_cptr *now, Time_cptr *until)
{
    if(txbuffer_ptr == txbuffer)
        return;

    iinic_idle();
    iinic_set_buffer(txbuffer, txbuffer_ptr - txbuffer);
    iinic_tx();

    DEBUG(  "[" TIME_FMT "] speaking until " TIME_FMT "\r\n",
            TIME_FMT_DATA(*now), TIME_FMT_DATA(*until));

    NOTICE( "[" TIME_FMT "] sending %d bytes\r\n",
            TIME_FMT_DATA(*now),
            txbuffer_ptr - txbuffer);

    if(!timed_poll(IINIC_TX_COMPLETE, until))
    {
        WARNING("[" TIME_FMT "] failed to send messages\r\n",
                TIME_FMT_DATA(*until));

        iinic_idle();
    }

    iinic_set_buffer(rxbuffer, SETTINGS_RXBUFFER_SIZE);
    iinic_rx();
}

#endif // __INITIALIZATION_H__
