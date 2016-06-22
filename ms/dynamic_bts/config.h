#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "iinic/iinic.h"

enum Settings
{
    // GLOBAL
    SETTINGS_MAX_NODES                  = 18,
    SETTINGS_MAX_EDGES                  = 128,
    SETTINGS_REQUEST_QUEUE_SIZE         = 4,
    SETTINGS_MAX_HOP                    = 1,

    // RADIO
    SETTINGS_RADIO_BITRATE              = IINIC_BITRATE_115200,
    SETTINGS_RADIO_RX                   = IINIC_BW_200
                                        | IINIC_GAIN_20
                                        | IINIC_RSSI_73,
    SETTINGS_RXBUFFER_SIZE              = 256,
    SETTINGS_RADIO_TX                   = IINIC_DEVIATION_120
                                        | IINIC_POWER_175,
    SETTINGS_CONTROL_TXBUFFER_SIZE      = 256,
    SETTINGS_DATA_TXBUFFER_SIZE         = SETTINGS_RXBUFFER_SIZE, // = RXBUFFER

    // BTS
    SETTINGS_INITIALIZATION_FRAMES      = 64,
    SETTINGS_CONTROL_FRAME_TIME         = 131072LU,
    SETTINGS_DATA_FRAME_TIME            = 393216LU,

    // SYNCHRONIZATION
    SETTINGS_ROOT_TTL                   = 96,
    SETTINGS_SYNCHRONIZATION_TTL        = 255,

    SETTINGS_SYNCHRONIZATION_PERIOD     = 32,
    SETTINGS_SYNCHRONIZATION_POINTS     = 8,

    // NEIGHBOURHOOD
    SETTINGS_NEIGHBOURHOOD_TTL          = 16,

    // NEIGHBOURS
    SETTINGS_NEIGHBOUR_TTL              = 96,
    SETTINGS_NEIGHBOURS_PERIOD          = 64,
};

#define DEBUG_LEVEL 5

//#define STATIC_ROOT 0xFFFF

#endif // __CONFIG_H__
