#include <stdint.h>

#include "common.h"
#include "iinic_wrapper.h"
#include "usart_complex.h"
#include "messages/handlers.h"
#include "protocol/handlers.h"
#include "protocol/initialization.h"
#include "protocol/tdma.h"
#include "sensors_config.h"


uint8_t rxbuffer[SETTINGS_RXBUFFER_SIZE];
uint8_t txbuffer[SETTINGS_TXBUFFER_SIZE];
uint8_t *txbuffer_ptr;

uint8_t sensor_id;

uint16_t tdma_slot;

struct USARTData usart;

inline
uint8_t usart_get_packet_size(const uint8_t kind)
{
    return usart_get_builtin_packet_size(kind);
}


void iinic_main(void)
{
    txbuffer_ptr = txbuffer;

    // DEVICE SETUP
    sensor_id = get_sensor_id(iinic_mac);

    iinic_set_rx_knobs(SETTINGS_RADIO_RX);
    iinic_set_tx_knobs(SETTINGS_RADIO_TX);
    iinic_set_bitrate(SETTINGS_RADIO_BITRATE);

    // SETUP USART
    iinic_usart_is_complex();

    // SETUP LEDS
    iinic_led_off(IINIC_LED_GREEN);
    iinic_led_off(IINIC_LED_RED);

    Time time; time_get_now(&time);

    NOTICE( "\r\n\n[" TIME_FMT "] Hello World! My MAC is 0x%04x\r\n",
            TIME_FMT_DATA(time), iinic_mac);

    on_init(&time, MAIN_EVENT);

    // INITIALIZATION
    initialization_loop();

    // TDMA
    tdma_loop();
}
