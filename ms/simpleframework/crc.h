#ifndef __CRC_H__
#define __CRC_H__

#include <util/crc16.h>
#include <stdint.h>

inline
static uint16_t crc16(const uint8_t *buf, uint16_t len)
{
    uint16_t crc = 0xFFFF;
    while(len) {
        crc = _crc16_update(crc, *buf ++);
        -- len;
    }

    return crc;
}

#endif // __CRC_H__
