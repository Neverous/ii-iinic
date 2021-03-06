#ifndef __SENSORS_CONFIG_H__
#define __SENSORS_CONFIG_H__

#include <avr/pgmspace.h>
#include "macs.h"

typedef struct SensorConfig
{
    uint16_t    macaddr;
    uint8_t     backoff_ppb;
} SensorConfig;

extern const SensorConfig sensor_config[SENSORS_COUNT];

inline
uint8_t sensor_config_get_backoff_ppb(uint8_t sid)
{
    return pgm_read_byte(&sensor_config[sid].backoff_ppb);
}

inline
uint8_t sensor_config_get_macaddr(uint8_t sid)
{
    return pgm_read_word(&sensor_config[sid].macaddr);
}

#endif // __SENSORS_CONFIG_H__
