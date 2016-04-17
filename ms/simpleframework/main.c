#include "config.h"
#include "iinic/iinic.h"
#include "iinic_patch.h"
#include "messages/enabled.h"
#include "protocol/handlers.h"
#include "initialization.h"
#include "tdma.h"

uint8_t rxbuffer[SETTINGS_RXBUFFER_SIZE];
uint8_t txbuffer[SETTINGS_TXBUFFER_SIZE];
uint8_t *txbuffer_ptr;

void iinic_main(void)
{
    txbuffer_ptr = txbuffer;

    // DEVICE SETUP
    iinic_set_rx_knobs(SETTINGS_RX);
    iinic_set_tx_knobs(SETTINGS_TX);
    iinic_set_bitrate(SETTINGS_BITRATE);

    iinic_usart_is_debug();

    iinic_led_off(IINIC_LED_GREEN);
    iinic_led_off(IINIC_LED_RED);

    iinic_timing time; iinic_get_now(&time);

    debug("\r\n\n[%lu] Hello World! My MAC is 0x%04x\r\n", *(uint32_t *) &time, iinic_mac);

    on_init(&time, MAIN_EVENT);

    // INITIALIZATION
    initialization_loop();

    // TDMA
    tdma_loop();
}
