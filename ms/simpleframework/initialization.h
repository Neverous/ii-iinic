#ifndef __INITIALIZATION_H__
#define __INITIALIZATION_H__

#include <stdlib.h>
#include "config.h"
#include "iinic/iinic.h"
#include "iinic_patch.h"
#include "protocol/handlers.h"

void initialization_listen_until(iinic_timing_cptr *until)
{
    while(iinic_timed_poll(IINIC_RX_COMPLETE, until))
    {
        handle_messages(&iinic_rx_timing, iinic_rx_rssi, rxbuffer, iinic_buffer_ptr, INITIALIZATION_EVENT);
        iinic_rx();
    }
}

void initialization_speak_until(iinic_timing_cptr *until)
{
    if(txbuffer_ptr == txbuffer)
        return;

    debug("[?] sending %d bytes\r\n", txbuffer_ptr - txbuffer);
    iinic_idle();
    iinic_set_buffer(txbuffer, txbuffer_ptr - txbuffer);
    iinic_tx();
    if(!iinic_timed_poll(IINIC_TX_COMPLETE, until))
    {
        debug("[?] failed to send messages\r\n");
        iinic_idle();
    }

    iinic_set_buffer(rxbuffer, SETTINGS_RXBUFFER_SIZE);
    iinic_rx();
}

void initialization_loop(void)
{
    iinic_idle();
    iinic_set_buffer(rxbuffer, SETTINGS_RXBUFFER_SIZE);
    iinic_rx();
    iinic_timing deadline; iinic_get_now(&deadline);
    debug("\r\n[%lu] :: Initializing\r\n", *(uint32_t *) &deadline);
    iinic_timing_add_32(
        &deadline,
        SETTINGS_INITIALIZATION_TIME * SETTINGS_INITIALIZATION_FRAME_TIME);

        on_init(&deadline, INITIALIZATION_EVENT);

    for(uint8_t r = 1; iinic_now_cmp(&deadline) < 0; ++ r)
    {
        iinic_timing frame_start; iinic_get_now(&frame_start);
        iinic_timing frame_deadline = frame_start;
        iinic_timing slot_start = frame_deadline;
        iinic_timing_align(&frame_deadline, SETTINGS_INITIALIZATION_FRAME_TIME);

            on_frame_start(&frame_start, &frame_deadline, INITIALIZATION_EVENT);

        debug("[%lu] :::: Frame %u\r\n", *(uint32_t *) &frame_deadline, r);

        iinic_timing_add_32(&slot_start,
                            random() % SETTINGS_INITIALIZATION_FRAME_TIME);

        initialization_listen_until(&slot_start);

            on_slot_start(&slot_start, &frame_deadline, INITIALIZATION_EVENT);

        initialization_speak_until(&frame_deadline);

            on_slot_end(0, INITIALIZATION_EVENT);

        initialization_listen_until(&frame_deadline);

            on_frame_end(&frame_deadline, INITIALIZATION_EVENT);

        iinic_led_toggle(IINIC_LED_GREEN);
    }

    iinic_led_off(IINIC_LED_GREEN);
}

#endif // __INITIALIZATION_H__
