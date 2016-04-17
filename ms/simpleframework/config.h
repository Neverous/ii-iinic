#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "iinic/iinic.h"

enum settings
{
    SETTINGS_BITRATE                    = IINIC_BITRATE_115200,
    SETTINGS_FIRE_TTL                   = 32,
    SETTINGS_INITIALIZATION_FRAME_TIME  = 1048576LU,
    SETTINGS_INITIALIZATION_TIME        = 30,
    SETTINGS_MAX_NEIGHBOURS             = 32,
    SETTINGS_NEIGHBOURS_TTL             = 4,
    SETTINGS_ROOT_TTL                   = 255,
    SETTINGS_RX                         = IINIC_RSSI_103 | IINIC_GAIN_20 | IINIC_BW_270,
    SETTINGS_RXBUFFER_SIZE              = 768,
    SETTINGS_TDMA_FRAME_TIME            = 1048576LU,
    SETTINGS_TX                         = IINIC_POWER_75 | IINIC_DEVIATION_240,
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

#endif // __CONFIG_H__
