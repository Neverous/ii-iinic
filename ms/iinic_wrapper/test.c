#include <stdint.h>
#include <string.h>

#include "iinic_wrapper.h"
#include "usart_complex.h"

struct USARTData usart;

uint8_t usart_get_packet_size(const uint8_t kind)
{
    return usart_get_builtin_packet_size(kind);
}

void iinic_main(void)
{
#if 0
    // USART IS DEBUG
    iinic_usart_is_debug();
#else

    // USART IS COMPLEX
    iinic_usart_is_complex();
    for(uint16_t i = 0; i < 1000; ++ i)
        debug("%03u: %03u %03u %016u\r\n", i, usart.out.head, usart.out.cap, usart.out.tail);

    do
    {
        usart_commit();
        wdt_reset();
    }
    while(usart_pending_write());

    debug("%03u: %03u %03u %016u\r\n", 0, usart.out.head, usart.out.cap, usart.out.tail);

    do
    {
        usart_commit();
        wdt_reset();
    }
    while(usart_pending_write());
#endif
}
