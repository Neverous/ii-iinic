#ifndef __IINIC_PATCH_H__
#define __IINIC_PATCH_H__

#include "config.h"

void iinic_timing_align(iinic_timing *current, int32_t alignment)
{
    uint32_t *low = (uint32_t *) current;
    uint16_t *high = (uint16_t *) (low + 1);

    if(alignment == 0)
        return;

    int32_t remainder = *low % alignment;
    if(remainder == 0)
        return;

    uint32_t previous = *low;
    *low += alignment - remainder;
    if(previous > *low)
        *high += 1;
}

typedef const iinic_timing const iinic_timing_cptr;

#endif // __IINIC_PATCH_H__
