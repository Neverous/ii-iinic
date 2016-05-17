#include <stdint.h>

#define DEBUG_LEVEL 7

#include "iinic_wrapper.h"

uint8_t buffer[512];

void iinic_main(void)
{
    // DEVICE SETUP
    iinic_set_rx_knobs(IINIC_BW_270 | IINIC_GAIN_20 | IINIC_RSSI_91);
    iinic_set_tx_knobs(IINIC_DEVIATION_240 | IINIC_POWER_100);
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
    iinic_set_buffer(buffer, 512);
    iinic_rx();

    uint32_t count = 0;
    while(true)
    {
        Time now; time_get_now(&now);

        iinic_infinite_poll(IINIC_RX_COMPLETE);
        iinic_rx();

        ++ count;
        iinic_led_toggle(IINIC_LED_RED);
        DEBUG(  "[" TIME_FMT "] read %d\r\n", TIME_FMT_DATA(now), count);
    }
}
