#ifndef __TDMA_H__
#define  __TDMA_H__

#include "config.h"
#include "iinic_wrapper.h"
#include "protocol/handlers.h"

uint16_t tdma_slot;

void tdma_listen_until(Time_cptr *now, Time_cptr *until);
void tdma_speak_until(Time_cptr *now, Time_cptr *until);
void tdma_loop(void);


void tdma_loop(void)
{
    iinic_idle();
    iinic_set_buffer(rxbuffer, SETTINGS_RXBUFFER_SIZE);
    iinic_rx();

    tdma_slot = random() % SETTINGS_TDMA_SLOTS;
    Time loop_start; time_get_now(&loop_start);

    NOTICE("\r\n[" TIME_FMT "] :: Running TDMA\r\n", TIME_FMT_DATA(loop_start));
    DEBUG(  "[" TIME_FMT "]     frame_time=%lu\r\n",
            TIME_FMT_DATA(loop_start), SETTINGS_TDMA_FRAME_TIME);

    DEBUG(  "[" TIME_FMT "]     slot_time=%lu\r\n",
            TIME_FMT_DATA(loop_start),
            SETTINGS_TDMA_FRAME_TIME / SETTINGS_TDMA_SLOTS);

    DEBUG(  "[" TIME_FMT "]   slots_count=%d\r\n",
            TIME_FMT_DATA(loop_start), SETTINGS_TDMA_SLOTS);

    DEBUG(  "[" TIME_FMT "]   tdma_slot=%u\r\n",
            TIME_FMT_DATA(loop_start), tdma_slot);

    on_init(0, TDMA_EVENT);

    while(true)
    {
        Time frame_start; time_get_now(&frame_start);
        Time frame_deadline = frame_start;
        time_align(&frame_deadline, SETTINGS_TDMA_FRAME_TIME);

        on_frame_start(&frame_start, &frame_deadline, TDMA_EVENT);

        Time slot_start = frame_deadline;
        time_add_i32(
            &slot_start,
            - (int32_t) tdma_slot *
                (SETTINGS_TDMA_FRAME_TIME / SETTINGS_TDMA_SLOTS));

        Time slot_end = slot_start;
        time_add_i32(   &slot_end,
                        SETTINGS_TDMA_FRAME_TIME / SETTINGS_TDMA_SLOTS);

        tdma_listen_until(&frame_start, &slot_start);

        on_slot_start(&slot_start, &slot_end, TDMA_EVENT);

        tdma_speak_until(&slot_start, &slot_end);

        on_slot_end(&slot_end, TDMA_EVENT);

        tdma_listen_until(&slot_end, &frame_deadline);

        on_frame_end(&frame_deadline, TDMA_EVENT);

        iinic_led_toggle(IINIC_LED_GREEN);
    }

    iinic_led_off(IINIC_LED_GREEN);
}

void tdma_listen_until(Time_cptr *now, Time_cptr *until)
{
    DEBUG(  "[" TIME_FMT "] listening until " TIME_FMT "\r\n",
            TIME_FMT_DATA(*now), TIME_FMT_DATA(*until));

    while(timed_poll(IINIC_RX_COMPLETE, until))
    {
        handle_messages((Time_cptr *) &iinic_rx_timing, iinic_rx_rssi,
                        rxbuffer, iinic_buffer_ptr, TDMA_EVENT);
        iinic_rx();
    }
}

void tdma_speak_until(Time_cptr *now, Time_cptr *until)
{
    if(txbuffer_ptr == txbuffer)
        return;

    iinic_idle();
    iinic_set_buffer(txbuffer, txbuffer_ptr - txbuffer);
    iinic_tx();

    DEBUG(  "[" TIME_FMT "] speaking until " TIME_FMT "\r\n",
            TIME_FMT_DATA(*now), TIME_FMT_DATA(*until));

    NOTICE( "[" TIME_FMT "] sending %d bytes\r\n",
            TIME_FMT_DATA(*now), txbuffer_ptr - txbuffer);

    uint32_t count = 0;
    while(timed_poll(IINIC_TX_COMPLETE, until))
    {
        count ++;
        iinic_tx();
    }

    if(!count)
    {
        WARNING("[" TIME_FMT "] failed to send any messages\r\n",
                TIME_FMT_DATA(*until));
    }

    else
    {
        DEBUG(  "[" TIME_FMT "] sent %lu times\r\n",
                TIME_FMT_DATA(*until), count);
    }

    iinic_idle();
    iinic_set_buffer(rxbuffer, SETTINGS_RXBUFFER_SIZE);
    iinic_rx();
}

#endif // __TDMA_H__
