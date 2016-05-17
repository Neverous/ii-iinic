#include <stdint.h>
#include <string.h>

#define DEBUG_LEVEL 7

#include "iinic_wrapper.h"

uint8_t buffer[512];

void iinic_main(void)
{
    // DEVICE SETUP
    iinic_set_rx_knobs(IINIC_BW_270 | IINIC_GAIN_20 | IINIC_RSSI_91);
    iinic_set_tx_knobs(IINIC_DEVIATION_240 | IINIC_POWER_175);
    iinic_set_bitrate(IINIC_BITRATE_115200);

    // SETUP USART
    iinic_usart_is_debug();

    // SETUP LEDS
    iinic_led_off(IINIC_LED_GREEN);
    iinic_led_off(IINIC_LED_RED);

    Time time; time_get_now(&time);

    NOTICE( "\r\n\n[" TIME_FMT "] Hello World! My MAC is 0x%04x\r\n",
            TIME_FMT_DATA(time), iinic_mac);

    iinic_idle();
    strcpy((char *) buffer, "tuner message");
    iinic_set_buffer(buffer, 14);

    uint32_t count = 0;
    while(true)
    {
        iinic_tx();
        Time now; time_get_now(&now);
        Time until = now;
        time_add_i32(&until, 1048576);

        iinic_infinite_poll(IINIC_TX_COMPLETE);
        iinic_led_toggle(IINIC_LED_RED);

        ++ count;
        timed_poll(0, &until);
        DEBUG(  "[" TIME_FMT "] sent %d\r\n", TIME_FMT_DATA(now), count);

        iinic_led_toggle(IINIC_LED_GREEN);
    }
}
