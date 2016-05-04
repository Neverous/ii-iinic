#ifndef __SENSORS_CONFIG_H__
#define __SENSORS_CONFIG_H__

#include <avr/pgmspace.h>
#include "macs.h"

typedef struct SensorConfig
{
    uint8_t seed;
} SensorConfig;

extern const SensorConfig sensor_config[SENSORS_COUNT];

inline
uint8_t sensor_config_get_seed(uint8_t sid)
{
    return pgm_read_byte(&sensor_config[sid].seed);
}

#endif // __SENSORS_CONFIG_H__
