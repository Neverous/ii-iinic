#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stdint.h>
#include "iinic/iinic.h"

#define DEBUG_LEVEL 3

enum settings
{
    SETTINGS_BITRATE                    = IINIC_BITRATE_115200,
    SETTINGS_DISCOVERY_PERIOD           = 1,
    SETTINGS_INITIALIZATION_FRAME_TIME  = 2097152LU,
    SETTINGS_INITIALIZATION_TIME        = 30,
    SETTINGS_MAX_NEIGHBOURS             = 32,
    SETTINGS_NEIGHBOURS_TTL             = 4,
    SETTINGS_ROOT_TIME                  = 16,
    SETTINGS_ROOT_TTL                   = 128,
    SETTINGS_RX                         = IINIC_RSSI_73
                                        | IINIC_GAIN_0
                                        | IINIC_BW_270,

    SETTINGS_RXBUFFER_SIZE              = 768,
    SETTINGS_SYNCHRONIZATION_PERIOD     = 8,
    SETTINGS_SYNCHRONIZATION_POINTS     = 8,
    SETTINGS_TDMA_FRAME_TIME            = 2097152LU,
    SETTINGS_TDMA_SLOTS                 = 128,
    SETTINGS_TX                         = IINIC_POWER_0
                                        | IINIC_DEVIATION_240,

    SETTINGS_TXBUFFER_SIZE              = 256,
};

extern uint8_t rxbuffer[SETTINGS_RXBUFFER_SIZE];
extern uint8_t txbuffer[SETTINGS_TXBUFFER_SIZE];
extern uint8_t *txbuffer_ptr;

#define MESSAGES_CONFIGURATION                                              \
    REGISTER_MESSAGE(DISCOVERY,         Discovery,          CONSTANT,   0)  \
    REGISTER_MESSAGE(SYNCHRONIZATION,   Synchronization,    CONSTANT,   1)  \
    REGISTER_MESSAGE(NEIGHBOURS,        Neighbours,         CONSTANT,   2)

enum EventOptions
{
    MAIN_EVENT              = 0,
    INITIALIZATION_EVENT    = 1,
    TDMA_EVENT              = 2,
};

#define _unused __attribute__((unused))

#include "debug.h"
#include "iinic_wrapper.h"

#endif // __CONFIG_H__
