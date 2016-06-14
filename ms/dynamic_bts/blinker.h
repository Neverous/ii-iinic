#ifndef __BLINKER_H__
#define __BLINKER_H__

#include "common.h"
#include "protocol/messages.h"

inline
static
void blink(Time_cptr time)
{
    uint8_t MAIN_LED    = IINIC_LED_GREEN;
    uint8_t SUPPORT_LED = IINIC_LED_RED;

#ifndef STATIC_ROOT
    if(synchronization.root.macaddr == device_macaddr)
#else
    if(STATIC_ROOT == device_macaddr)
#endif
    {
        MAIN_LED    = IINIC_LED_RED;
        SUPPORT_LED = IINIC_LED_GREEN;
    }

    if((time->low / (   SETTINGS_CONTROL_FRAME_TIME +
                        SETTINGS_DATA_FRAME_TIME)) % 2)
        iinic_led_off(MAIN_LED);

    else
        iinic_led_on(MAIN_LED);

    if(neighbourhood.ttl)
        iinic_led_on(SUPPORT_LED);

    else
        iinic_led_off(SUPPORT_LED);
}

#endif // __BLINKER_H__
