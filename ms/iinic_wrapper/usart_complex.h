#ifndef __USART_COMPLEX__
#define __USART_COMPLEX__

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/wdt.h>

#include "iinic_wrapper.h"
#include "io_buffer.h"
#include "usart_structs.h"

#undef debug
#define debug debug_complex

void iinic_usart_is_complex(void);
uint8_t usart_push(const uint8_t *buf, const uint8_t len);
void usart_push_block(const uint8_t *buf, uint8_t len);
uint8_t usart_get(uint8_t *buf, const uint8_t len);
void usart_get_block(uint8_t *buf, uint8_t len);
void usart_commit(void);
uint8_t usart_pending_write(void);
void debug_complex(const char *fmt, ...);
extern uint8_t usart_get_packet_size(const uint8_t kind);


inline
void iinic_usart_is_complex(void)
{
    // usart: 230400, 8n1, rx interrupt, tx idle interrupt
    UBRRH = 0;
    UBRRL = 3;
    UCSRC = _BV(URSEL) | _BV(UCSZ1) | _BV(UCSZ0);
    UCSRB = _BV(RXCIE) | _BV(UDRIE) | _BV(RXEN) | _BV(TXEN);
}

inline
uint8_t usart_push(const uint8_t *buf, const uint8_t len)
{
    return io_buffer_push(&usart.out, buf, len);
}

inline
void usart_push_block(const uint8_t *buf, uint8_t len)
{
    while(len)
    {
        uint8_t sent = usart_push(buf, len);
        if(!sent)
            usart_commit();

        else
        {
            len -= sent;
            buf += sent;
        }

        wdt_reset();
    }

    usart_commit();
}

inline
uint8_t usart_get(uint8_t *buf, const uint8_t len)
{
    return io_buffer_get(&usart.in, buf, len);
}

inline
void usart_get_block(uint8_t *buf, uint8_t len)
{
    while(len)
    {
        uint8_t read = usart_get(buf, len);
        if(read)
        {
            len -= read;
            buf += read;
        }

        wdt_reset();
    }
}

inline
void usart_commit(void)
{
    io_buffer_commit(&usart.out);
    UCSRB |= _BV(UDRIE);
}

inline
uint8_t usart_pending_write(void)
{
    return io_buffer_used_size(&usart.out) > 0;
}

inline
void debug_complex(const char *fmt, ...)
{
    char buffer[128] = {};
    va_list args;
    va_start(args, fmt);
    uint8_t buf_size = vsnprintf_P( buffer + 1, 127 - sizeof(USARTDebug),
                                    fmt, args);
    va_end(args);

    if(buf_size > 127 - sizeof(USARTDebug))
        buf_size = 127 - sizeof(USARTDebug);

    uint8_t *buf = (uint8_t *) buffer;
    while(buf_size >= sizeof(USARTDebug))
    {
        *buf = USART_DEBUG;
        usart_push_block(buf, sizeof(USARTDebug));
        buf += sizeof(USARTDebug) - 1;
        buf_size -= sizeof(USARTDebug) - 1;
    }

    if(buf_size > 0)
    {
        *buf = USART_DEBUG;
        usart_push_block(buf, sizeof(USARTDebug));
    }
}

///////////////////////////////////////////////////////////////////////////////

// send
ISR(USART_UDRE_vect)
{
    uint8_t byte = 0;
    if(!io_buffer_get_one(&usart.out, &byte))
    {
        UCSRB &= ~_BV(UDRIE);
        return;
    }

    UDR = byte;
}

// receive
ISR(USART_RXC_vect)
{
    static uint8_t bytes_left = 1;
    uint8_t data = UDR;
    io_buffer_push_one(&usart.in, data);

    if(!-- bytes_left)
    {
        io_buffer_commit(&usart.in);
        bytes_left = usart_get_packet_size(data) - 1;
    }
}

#endif // __USART_COMPLEX__
