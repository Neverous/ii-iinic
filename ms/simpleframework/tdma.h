#ifndef __TDMA_H__
#define  __TDMA_H__

#include "config.h"
#include "iinic/iinic.h"
#include "iinic_patch.h"
#include "protocol/handlers.h"

uint16_t tdma_slot;

void tdma_listen_until(const iinic_timing const *until)
{
    iinic_set_buffer(rxbuffer, SETTINGS_RXBUFFER_SIZE);
    iinic_rx();
    uint8_t ret = 0;
    while((ret = iinic_timed_poll(IINIC_RX_COMPLETE, until)))
    {
        handle_messages(iinic_rx_rssi, rxbuffer, iinic_buffer_ptr, TDMA_EVENT);
        iinic_rx();
    }

    if(!ret)
        iinic_idle();
}

void tdma_speak_until(const iinic_timing const *until)
{
    if(txbuffer_ptr == txbuffer)
        return;

    debug("[?] sending %d bytes\r\n", txbuffer_ptr - txbuffer);
    iinic_set_buffer(txbuffer, txbuffer_ptr - txbuffer);
    iinic_tx();
    if(!iinic_timed_poll(IINIC_TX_COMPLETE, until))
    {
        debug("[?] failed to send messages\r\n");
        iinic_idle();
    }
}

void tdma_loop(void)
{

    tdma_slot = random() % SETTINGS_MAX_NEIGHBOURS;
    debug("\r\n[?] :: Running TDMA\r\n");
    debug("[?]   frame_time=%lu\r\n", SETTINGS_TDMA_FRAME_TIME);
    debug(  "[?]   slot_time=%lu\r\n",
            SETTINGS_TDMA_FRAME_TIME / SETTINGS_MAX_NEIGHBOURS);

    debug("[?]   slots_count=%d\r\n", SETTINGS_MAX_NEIGHBOURS);
    debug("[?]   tdma_slot=%u\r\n", tdma_slot);
    on_init(TDMA_EVENT);

    while(true)
    {
        iinic_timing frame_deadline; iinic_get_now(&frame_deadline);

            on_frame_start(&frame_deadline, TDMA_EVENT);

        iinic_timing_align(&frame_deadline, SETTINGS_TDMA_FRAME_TIME);

        iinic_timing slot_start = frame_deadline;
        iinic_timing_add_32(
            &slot_start,
            - (int32_t) tdma_slot * (SETTINGS_TDMA_FRAME_TIME / SETTINGS_MAX_NEIGHBOURS));

        iinic_timing slot_end = slot_start;
        iinic_timing_add_32(&slot_end,
                            SETTINGS_TDMA_FRAME_TIME / SETTINGS_MAX_NEIGHBOURS);

        tdma_listen_until(&slot_start);

            on_slot_start(&slot_start,TDMA_EVENT);

        tdma_speak_until(&slot_end);

            on_slot_end(&slot_end, TDMA_EVENT);

        tdma_listen_until(&frame_deadline);

            on_frame_end(&frame_deadline, TDMA_EVENT);

        iinic_led_toggle(IINIC_LED_GREEN);
    }
}

#endif // __TDMA_H__
