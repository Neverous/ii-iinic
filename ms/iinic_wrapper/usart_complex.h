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
uint16_t usart_crc16(const uint8_t len);
uint8_t usart_peek(const uint8_t **buf, const uint8_t len);
uint8_t usart_get(uint8_t *buf, const uint8_t len);
void usart_get_block(uint8_t *buf, uint8_t len);
uint8_t usart_pop(const uint8_t len);
void usart_commit(void);
uint8_t usart_pending_read(void);
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
uint16_t usart_crc16(const uint8_t len)
{
    return io_buffer_crc16(&usart.in, len);
}

inline
uint8_t usart_peek(const uint8_t **buf, const uint8_t len)
{
    return io_buffer_peek(&usart.in, len, buf);
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
uint8_t usart_pop(const uint8_t len)
{
    return io_buffer_pop(&usart.in, len);
}

inline
void usart_commit(void)
{
    io_buffer_commit(&usart.out);
    UCSRB |= _BV(UDRIE);
}

inline
uint8_t usart_pending_read(void)
{
    return io_buffer_used_size(&usart.in);
}

inline
uint8_t usart_pending_write(void)
{
    return io_buffer_used_size(&usart.out);
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
        uint16_t crc = crc16(buf, sizeof(USARTDebug));
        usart_push_block(buf, sizeof(USARTDebug));
        usart_push_block((uint8_t *) &crc, 2);
        buf += sizeof(USARTDebug) - 1;
        buf_size -= sizeof(USARTDebug) - 1;
    }

    if(buf_size > 0)
    {
        *buf = USART_DEBUG;
        uint16_t crc = crc16(buf, sizeof(USARTDebug));
        usart_push_block(buf, sizeof(USARTDebug));
        usart_push_block((uint8_t *) &crc, 2);
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
    static uint8_t bytes_left = 0;
    uint8_t data = UDR;

    if(!bytes_left)
        bytes_left = usart_get_packet_size(data);

    io_buffer_push_one(&usart.in, data);
    if(!-- bytes_left)
    {
        io_buffer_commit(&usart.in);
        iinic_signal();
    }
}

#endif // __USART_COMPLEX__
