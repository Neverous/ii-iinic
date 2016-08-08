#ifndef COMMON_H
#define COMMON_H

#include <cstdint>

// Dla kompatybilności z kodem z AVR

#define SETTINGS_MAX_NODES  16 // sync with ../config.h

#ifndef _MSC_VER
#define __unused__          __attribute__((unused))
#else
#pragma warning(disable: 4100)  // unreferenced formal parameter
#pragma warning(disable: 4200)  // nonstandard extension used: zero-sized array in struct/union
#define __unused__
#endif

#pragma pack(push, 1)

struct Time
{
    uint32_t low;
    uint16_t high;
};

#pragma pack(pop)

typedef const Time * const Time_cptr;
typedef const uint8_t * const uint8_t_cptr;

uint16_t crc16(const uint8_t *buf, uint16_t len)
{
    uint16_t crc = 0xFFFF;
    while(len)
    {
        crc ^= *buf;
        for(int i = 0; i < 8; ++ i)
        {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xA001;
            else
                crc = (crc >> 1);
        }
        ++ buf;
        -- len;
    }

    return crc;
}

#endif // COMMON_H
