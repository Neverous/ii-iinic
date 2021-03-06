#ifndef __IINIC_WRAPPER_H__
#define __IINIC_WRAPPER_H__

#include <stdlib.h>
#include <string.h>
#include <util/crc16.h>

#include "iinic/iinic.h"

#define __unused__ __attribute__((unused))
#define must_read(x)    (*(const volatile typeof(x) *)&x)
#define must_write(x)   (*(volatile typeof(x) *)&x)

#define TIME_FMT            "%05u:%010lu"
#define TIME_FMT_COMPACT    "%u:%lu"
#define TIME_FMT_DATA(time) (time).high, (time).low
#define TIME_NULL           "00000:0000000000"
#define TIME_NULL_COMPACT   "0:0"

#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL 5
#endif

#define DEBUG(...)      {if(DEBUG_LEVEL > 4) debug(__VA_ARGS__);}
#define NOTICE(...)     {if(DEBUG_LEVEL > 3) debug(__VA_ARGS__);}
#define INFO(...)       {if(DEBUG_LEVEL > 2) debug(__VA_ARGS__);}
#define WARNING(...)    {if(DEBUG_LEVEL > 1) debug(__VA_ARGS__);}
#define ERROR(...)      {if(DEBUG_LEVEL > 0) debug(__VA_ARGS__);}

#define max(a, b)   ((a) > (b) ? (a) : (b))
#define min(a, b)   ((a) < (b) ? (a) : (b))

typedef union
{
    iinic_timing t;
    struct
    {
        uint32_t low;
        uint16_t high;
    };
} Time;

typedef const uint8_t * const uint8_t_cptr;
typedef const Time * const Time_cptr;
typedef const iinic_timing * const iinic_timing_cptr;


void time_align(Time *current, int32_t alignment);
void time_add_i32(Time *time, int32_t value);
int8_t time_cmp(Time_cptr time1, Time_cptr time2);
int8_t time_cmp_now(Time_cptr time);
void time_get_now(Time *time);
uint8_t timed_poll(uint8_t mask, Time_cptr until);
void time_add(Time *a, Time_cptr b);
void time_sub(Time *a, Time_cptr b);

uint16_t crc16(uint8_t_cptr buf, uint16_t len);

uint8_t _scale_rssi(uint16_t rssi);
void swap(uint8_t *dst, uint8_t *src, uint8_t size);
void swap_lowmem(uint8_t *dst, uint8_t *src, uint8_t size);
void shuffle(uint8_t *array, uint8_t count, uint8_t size);


inline
void time_align(Time *current, int32_t alignment)
{
    if(alignment == 0)
        return;

    int32_t remainder = current->low % alignment;
    if(remainder == 0)
        return;

    uint32_t previous = current->low;
    current->low += alignment - remainder;
    if(previous > current->low)
        current->high += 1;
}

inline
void time_add_i32(Time *time, int32_t value)
{
    iinic_timing_add_32((iinic_timing *) time, value);
}

inline
int8_t time_cmp(Time_cptr time1, Time_cptr time2)
{
    return iinic_timing_cmp((iinic_timing_cptr) time1,
                            (iinic_timing_cptr) time2);
}

inline
int8_t time_cmp_now(Time_cptr time)
{
    return iinic_now_cmp((iinic_timing_cptr) time);
}

inline
void time_get_now(Time *time)
{
    iinic_get_now((iinic_timing *) time);
}

inline
uint8_t timed_poll(uint8_t mask, Time_cptr until)
{
    return iinic_timed_poll(mask, (iinic_timing_cptr) until);
}

inline
void time_add(Time *a, Time_cptr b)
{
    iinic_timing_add((iinic_timing *) a, (iinic_timing_cptr) b);
}

inline
void time_sub(Time *a, Time_cptr b)
{
    iinic_timing_sub((iinic_timing *) a, (iinic_timing_cptr) b);
}

inline
uint16_t crc16(const uint8_t *buf, uint16_t len)
{
    uint16_t crc = 0xFFFF;
    while(len) {
        crc = _crc16_update(crc, *buf ++);
        -- len;
    }

    return crc;
}

inline
uint8_t _scale_rssi(uint16_t rssi)
{
    if(rssi < 180)
        return 0;

    rssi -= 180;
    if(rssi > 255)
        return 255;

    return rssi;
}

void swap(uint8_t *dst, uint8_t *src, uint8_t size)
{
    if(dst == src)
        return;

    uint8_t temp[size];
    memcpy(temp, src, size);
    memcpy(src, dst, size);
    memcpy(dst, temp, size);
}

inline
void swap_lowmem(uint8_t *dst, uint8_t *src, uint8_t size)
{
    if(dst == src)
        return;

    while(size)
    {
        uint8_t temp = *src;
        *src ++ = *dst;
        *dst ++ = temp;
        -- size;
    }
}

inline
void shuffle(uint8_t *array, uint8_t count, uint8_t size)
{
    while(count > 1)
    {
        uint8_t position = random() % count;
        -- count;
        swap(array + size * count, array + size * position, size);
    }
}

#endif // __IINIC_WRAPPER_H__
