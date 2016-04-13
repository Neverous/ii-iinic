#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <avr/interrupt.h>

#include "iinic/iinic.h"

#include "protocol.h"

enum settings_t
{
    MAX_PACKET_LENGTH       = 32,
    MAX_RXBUFFER_LENGTH     = 1536,
    MAX_NEIGHBOURS_COUNT    = 16,
    TTL_THRESHOLD           = 256,
    MAX_ROUNDS              = 64,
}; // enum settings_t


uint8_t rxbuffer[MAX_RXBUFFER_LENGTH];
uint8_t *rxbuffer_ptr;
uint8_t txbuffer[MAX_PACKET_LENGTH];
uint8_t *txbuffer_ptr;

struct neighbour
{
    uint16_t    macaddr;
    uint8_t     ttl;
    uint16_t    rssi;
}; // struct neighbour

struct neighbour neighbour[MAX_NEIGHBOURS_COUNT];

inline
static void show_neighbours()
{
    debug("Active neighbours:\r\n");
    struct neighbour *current = neighbour;
    for(uint8_t n = 0; n < MAX_NEIGHBOURS_COUNT; ++ n, ++ current)
        if(current->ttl)
            debug("  MAC: 0x%04x RSSI: %d TTL: %d\r\n", current->macaddr, current->rssi, current->ttl);

    debug("\r\n");
}

inline
static struct neighbour *find_neighbour(uint16_t _macaddr)
{
    struct neighbour *current = neighbour;
    for(uint8_t n = 0; n < MAX_NEIGHBOURS_COUNT; ++ n, ++ current)
        if(current->ttl && current->macaddr == _macaddr)
            return current;

    return 0;
}

inline
static struct neighbour *find_slot()
{
    struct neighbour *current = neighbour;
    for(uint8_t n = 0; n < MAX_NEIGHBOURS_COUNT; ++ n, ++ current)
        if(!current->ttl)
            return current;

    return 0;
}

inline
static void update_neighbour(uint16_t _macaddr, uint16_t rssi)
{
    struct neighbour *current = find_neighbour(_macaddr);
    if(!current)
    {
        current = find_slot();
        if(!current)
        {
            debug("Too many neighbours!\r\n");
            return;
        }

        debug("New neighbour: 0x%04x\r\n", _macaddr);
        current->macaddr = _macaddr;
    }

    current->ttl = 255;
    current->rssi = rssi;
}

inline
static void check_neighbours()
{
    struct neighbour *current = neighbour;
    for(uint8_t n = 0; n < MAX_NEIGHBOURS_COUNT; ++ n, ++ current)
        if(current->ttl)
            -- current->ttl;
}

void __attribute__((noreturn)) iinic_main()
{
    iinic_set_rx_knobs(IINIC_RSSI_91 | IINIC_GAIN_20 | IINIC_BW_270);
    iinic_set_tx_knobs(IINIC_POWER_75 | IINIC_DEVIATION_240);
    iinic_set_bitrate(IINIC_BITRATE_57600);

    iinic_usart_is_debug();

    debug("\r\n\r\nHello, world, my mac is %04x\r\n", iinic_mac);
    debug("Time to work!\r\n");

    uint16_t round = 0;
    uint16_t my_slot = random() % MAX_ROUNDS;
    debug("My slot: %d\r\n", my_slot);

    iinic_led_off(IINIC_LED_GREEN);

    txbuffer_ptr = hello_msg_write(txbuffer, txbuffer + MAX_PACKET_LENGTH, iinic_mac);
    if(!txbuffer_ptr)
        __iinic_panic();

    iinic_set_buffer(rxbuffer, MAX_RXBUFFER_LENGTH);
    iinic_rx();
    while(1)
    {
        ++ round;
        if(round >= MAX_ROUNDS)
            round = 0;

        iinic_timing deadline;
        iinic_get_now(&deadline); iinic_timing_add_32(&deadline, 65000);

        iinic_led_toggle(IINIC_LED_GREEN);
        if(!round)
            show_neighbours();

        check_neighbours();

        if(round == my_slot)
        {
            iinic_idle();
            iinic_set_buffer(txbuffer, txbuffer_ptr - txbuffer);
            uint8_t once = 0;
            while(1)
            {
                iinic_tx();
                uint8_t ret = iinic_timed_poll(IINIC_TX_COMPLETE, &deadline);
                if(ret != IINIC_TX_COMPLETE)
                    break;

                ++ once;
            }

            if(!once)
                debug("Couldn't send even one hello message\r\n");

            debug("Sent hello %d times\r\n", once);
            iinic_idle();
            iinic_set_buffer(rxbuffer, MAX_PACKET_LENGTH);
            iinic_rx();
        }

        else
        {
            uint16_t neighbour_macaddr;
            uint16_t prev_macaddr = 0;
            uint8_t ret;
            uint8_t count = 0;;
            while((ret = iinic_timed_poll(IINIC_RX_COMPLETE, &deadline)))
            {
                uint16_t rssi = iinic_rx_rssi;
                if(hello_msg_read(rxbuffer, iinic_buffer_ptr, &neighbour_macaddr))
                {
                    if(prev_macaddr != neighbour_macaddr)
                    {
                        update_neighbour(neighbour_macaddr, rssi);
                        prev_macaddr = neighbour_macaddr;
                    }

                    ++ count;
                }

                iinic_rx();
            }

            if(count > 0)
                printf("Read %d hello messages\r\n", count);
        }
    }
}
