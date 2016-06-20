#include "common.h"
#include "protocol/bts.h"
#include "protocol/initialization.h"
#include "protocol/messages.h"

uint16_t    device_macaddr;
uint8_t     *control_txbuffer_ptr;
uint8_t     *data_txbuffer;
uint8_t     *data_txbuffer_ptr;
uint8_t     control_txbuffer[SETTINGS_CONTROL_TXBUFFER_SIZE];
uint8_t     rxbuffer[SETTINGS_RXBUFFER_SIZE];

struct NeighbourhoodData    neighbourhood;
struct NeighboursData       neighbours;
struct PingPongData         pingpong;
struct RequestData          request;
struct SynchronizationData  synchronization;
struct USARTData            usart;

void iinic_main(void)
{
    control_txbuffer_ptr    = control_txbuffer;
    data_txbuffer           = rxbuffer;
    data_txbuffer_ptr       = data_txbuffer;
    device_macaddr          = iinic_mac;

    neighbourhood.ttl = 0;

    neighbours.is_neighbour     = 0;
    neighbours.node[0].macaddr  = iinic_mac;
    neighbours.timer            = random();

    pingpong.mode   = PP_MODE_HIDDEN;
    pingpong.ttl    = 0;

    for(uint8_t n = 0; n < SETTINGS_MAX_NODES; ++ n)
        request.assignment[n].ttl = 0;

    synchronization.clock.seq_id    = 0;
#ifndef STATIC_ROOT
    synchronization.root.macaddr    = 0xFFFF;
    synchronization.root.ttl        = SETTINGS_INITIALIZATION_FRAMES;
#endif
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
