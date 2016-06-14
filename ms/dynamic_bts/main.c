#include "common.h"
#include "protocol/bts.h"
#include "protocol/initialization.h"

uint16_t    device_macaddr;
uint8_t     rxbuffer[SETTINGS_RXBUFFER_SIZE];
uint8_t     txbuffer[SETTINGS_TXBUFFER_SIZE];
uint8_t     *txbuffer_ptr;

struct NeighbourhoodData    neighbourhood;
struct SynchronizationData  synchronization;
struct USARTData            usart;

void iinic_main(void)
{
    device_macaddr  = iinic_mac;
    txbuffer_ptr    = txbuffer;


    neighbourhood.ttl = 0;

    synchronization.clock.seq_id    = 0;
    synchronization.root.macaddr    = 0xFFFF;
    synchronization.root.ttl        = SETTINGS_INITIALIZATION_FRAMES;
    synchronization.timer.counter   = random();

    // DEVICE SETUP
    iinic_set_rx_knobs(SETTINGS_RADIO_RX);
    iinic_set_tx_knobs(SETTINGS_RADIO_TX);
    iinic_set_bitrate(SETTINGS_RADIO_BITRATE);

    // Setup USART
    iinic_usart_is_complex();

    // Setup LEDs
    iinic_led_off(IINIC_LED_GREEN);
    iinic_led_off(IINIC_LED_RED);

    Time time; time_get_now(&time);
    INFO(   "\r\n\n" TIME_FMT "| |MAC=0x%04x\r\n",
            TIME_FMT_DATA(time), device_macaddr);

    // INITIALIZATION
    initialization_loop();

    // MAIN (BTS) LOOP
    bts_loop();
}
