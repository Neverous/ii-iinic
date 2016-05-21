#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stdint.h>

#include "iinic/iinic.h"

enum Settings
{
    // GLOBAL
    SETTINGS_MAX_SENSORS                = 16,

    // RADIO
    SETTINGS_RADIO_BITRATE              = IINIC_BITRATE_115200,
    SETTINGS_RADIO_RX                   = IINIC_BW_200
                                        | IINIC_GAIN_0
                                        | IINIC_RSSI_103,
    SETTINGS_RXBUFFER_SIZE              = 384,
    SETTINGS_RADIO_TX                   = IINIC_DEVIATION_120
                                        | IINIC_POWER_0,
    SETTINGS_TXBUFFER_SIZE              = 256,

    // INITIALIZATION
    SETTINGS_INITIALIZATION_FRAMES      = 64,
    SETTINGS_INITIALIZATION_FRAME_TIME  = 524288LU,

    // TDMA
    SETTINGS_TDMA_FRAME_TIME            = 524288LU,
    SETTINGS_TDMA_SLOTS                 = 16,
    SETTINGS_TDMA_SLOT_MARGIN           = 5,    // time margin in %

    // DISCOVERY
    SETTINGS_DISCOVERY_PERIOD           = 1,    // Disabled after initialization
                                                // works with synchronization
    SETTINGS_ROOT_TIME                  = 16,
    SETTINGS_ROOT_TTL                   = 255,

    // SYNCHRONIZATION
    SETTINGS_SYNCHRONIZATION_PERIOD     = 64,
    SETTINGS_SYNCHRONIZATION_POINTS     = 8,

    // NEIGHBOURS
    SETTINGS_NEIGHBOURS_SHOW_TIME       = 8,

    // BACKOFF
    SETTINGS_BACKOFF_PAYLOAD_SIZE       = 8,
    SETTINGS_BACKOFF_TRIES_LIMIT        = 8,

    // GATHER
    SETTINGS_GATHER_PERIOD              = 240,
    SETTINGS_GATHER_CYCLE               = 64,
    SETTINGS_GATHER_MESSAGE_TTL         = 32,
    SETTINGS_GATHER_REQUEST_TTL         = 16,
};

#define DEBUG_LEVEL                 3
#define SYNCHRONIZATION_FAST_SYNC   true

#define SETTINGS_MESSAGES_ENABLE                                            \
    REGISTER_MESSAGE(DISCOVERY,         Discovery,          CONSTANT,   0)  \
    REGISTER_MESSAGE(SYNCHRONIZATION,   Synchronization,    CONSTANT,   1)  \
    REGISTER_MESSAGE(NEIGHBOURS,        Neighbours,         CONSTANT,   2)  \
    REGISTER_MESSAGE(BACKOFF,           Backoff,            CONSTANT,   3)  \
    REGISTER_MESSAGE(BACKOFFACK,        BackoffAck,         CONSTANT,   4)  \
    REGISTER_MESSAGE(GATHER_REQUEST,    GatherRequest,      CONSTANT,   5)  \
    REGISTER_MESSAGE(GATHER,            Gather,             VARIABLE,   0)

#endif // __CONFIG_H__
