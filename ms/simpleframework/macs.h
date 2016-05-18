#ifndef __MACS_H__
#define __MACS_H__

#include <stdint.h>

enum KnownMAC
{
    SENSOR_1_MAC    = 0x5518,
    SENSOR_2_MAC    = 0xaa18,
    SENSOR_3_MAC    = 0x8f1e,
    SENSOR_4_MAC    = 0x6223,
    SENSOR_5_MAC    = 0x9428,
    SENSOR_6_MAC    = 0x832d,
    SENSOR_7_MAC    = 0x954b,
    SENSOR_8_MAC    = 0xf74e,
    SENSOR_9_MAC    = 0x7e80,
    SENSOR_10_MAC   = 0x169d,
    SENSOR_11_MAC   = 0x829e,
    SENSOR_12_MAC   = 0xa7a2,
    SENSOR_13_MAC   = 0x53b5,
    SENSOR_14_MAC   = 0x20c2,
    SENSOR_15_MAC   = 0x1fcd,
    SENSOR_16_MAC   = 0x5cff,
    SENSORS_COUNT   = 16,
};

inline
uint8_t get_sensor_id(uint16_t macaddr)
{
    switch(macaddr)
    {
        case SENSOR_1_MAC:
            return 0;

        case SENSOR_2_MAC:
            return 1;

        case SENSOR_3_MAC:
            return 2;

        case SENSOR_4_MAC:
            return 3;

        case SENSOR_5_MAC:
            return 4;

        case SENSOR_6_MAC:
            return 5;

        case SENSOR_7_MAC:
            return 6;

        case SENSOR_8_MAC:
            return 7;

        case SENSOR_9_MAC:
            return 8;

        case SENSOR_10_MAC:
            return 9;

        case SENSOR_11_MAC:
            return 10;

        case SENSOR_12_MAC:
            return 11;

        case SENSOR_13_MAC:
            return 12;

        case SENSOR_14_MAC:
            return 13;

        case SENSOR_15_MAC:
            return 14;

        case SENSOR_16_MAC:
            return 15;
    }

    return -1;
}

#endif // __MACS_H__
