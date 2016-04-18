#ifndef __IINIC_WRAPPER_H__
#define __IINIC_WRAPPER_H__

#include "config.h"
#include "iinic/iinic.h"

#define TIME_FMT            "%u:%lu"
#define TIME_FMT_DATA(time) (time).high, (time).low

typedef union
{
    iinic_timing t;
    struct
    {
        uint32_t low;
        uint16_t high;
    };
} Time;

typedef const Time const Time_cptr;
typedef const iinic_timing const iinic_timing_cptr;


void time_align(Time *current, int32_t alignment);
void time_add_i32(Time *time, int32_t value);
int8_t time_cmp_now(Time_cptr *time);
void time_get_now(Time *time);
uint8_t timed_poll(uint8_t mask, Time_cptr *until);
void time_add(Time *a, Time_cptr *b);
void time_sub(Time *a, Time_cptr *b);


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

void time_add_i32(Time *time, int32_t value)
{
    iinic_timing_add_32((iinic_timing *) time, value);
}

int8_t time_cmp_now(Time_cptr *time)
{
    return iinic_now_cmp((iinic_timing_cptr *) time);
}

void time_get_now(Time *time)
{
    iinic_get_now((iinic_timing *) time);
}

uint8_t timed_poll(uint8_t mask, Time_cptr *until)
{
    return iinic_timed_poll(mask, (iinic_timing_cptr *) until);
}

void time_add(Time *a, Time_cptr *b)
{
    iinic_timing_add((iinic_timing *) a, (iinic_timing_cptr *) b);
}

void time_sub(Time *a, Time_cptr *b)
{
    iinic_timing_sub((iinic_timing *) a, (iinic_timing_cptr *) b);
}

#endif // __IINIC_WRAPPER_H__
