#ifndef __IINIC_PATCH_H__
#define __IINIC_PATCH_H__

#include "config.h"

void iinic_timing_align(iinic_timing *current, int32_t alignment)
{
    uint32_t *access = (uint32_t *) current;
    if(alignment == 0)
        return;

    int32_t remainder = *access % alignment;
    if(remainder == 0)
        return;

    *access += alignment - remainder;
}

typedef const iinic_timing const iinic_timing_cptr;

#endif // __IINIC_PATCH_H__
