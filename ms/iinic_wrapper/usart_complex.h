#ifndef __USART_COMPLEX__
#define __USART_COMPLEX__

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <string.h>

#include "iinic_wrapper.h"
#include "io_buffer.h"
#include "usart_structs.h"

#undef debug
#define debug debug_complex

void iinic_usart_is_complex(void);
uint8_t usart_push(const uint8_t *buf, const uint8_t len);
void usart_push_block(const uint8_t *buf, uint8_t len, bool commit);
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


void iinic_usart_is_complex(void)
{
    // usart: 230400, 8n1, rx interrupt, tx idle interrupt
    UBRRH = 0;
    UBRRL = 3;
    UCSRC = _BV(URSEL) | _BV(UCSZ1) | _BV(UCSZ0);
    UCSRB = _BV(RXCIE) | _BV(UDRIE) | _BV(RXEN) | _BV(TXEN);
}

uint8_t usart_push(const uint8_t *buf, const uint8_t len)
{
    return io_buffer_push(&usart.out, buf, len);
}

void usart_push_block(const uint8_t *buf, uint8_t len, bool commit)
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

    if(commit)
        usart_commit();
}

uint16_t usart_crc16(const uint8_t len)
{
    return io_buffer_crc16(&usart.in, len);
}

uint8_t usart_peek(const uint8_t **buf, const uint8_t len)
{
    return io_buffer_peek(&usart.in, len, buf);
}

uint8_t usart_get(uint8_t *buf, const uint8_t len)
{
    return io_buffer_get(&usart.in, buf, len);
}

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

uint8_t usart_pop(const uint8_t len)
{
    return io_buffer_pop(&usart.in, len);
}

void usart_commit(void)
{
    io_buffer_commit(&usart.out);
    UCSRB |= _BV(UDRIE);
}

uint8_t usart_pending_read(void)
{
    return io_buffer_used_size(&usart.in);
}

uint8_t usart_pending_write(void)
{
    return io_buffer_used_size(&usart.out);
}

static uint16_t _debug_crc = 0xFFFF;
static uint8_t _debug_current_size = 0;

void _debug_push(const uint8_t *ptr, uint8_t bytes)
{
    // HEADER
    if(!_debug_current_size)
    {
        uint8_t k = USART_DEBUG;
        usart_push_block(&k, 1, true);
        _debug_crc = _crc16_update(_debug_crc, k);
    }

    if(bytes > 31 - _debug_current_size)
    {
        uint8_t space_left = 31 - _debug_current_size;
        usart_push_block(ptr, space_left, false);
        while(space_left)
        {
            _debug_crc = _crc16_update(_debug_crc, *ptr ++);
            -- space_left;
            -- bytes;
        }

        // FOOTER
        usart_push_block((uint8_t *) &_debug_crc, 2, true);
        _debug_current_size = 0;
        _debug_crc = 0xFFFF;

        // HEADER
        space_left = USART_DEBUG;
        usart_push_block(&space_left, 1, false);
        _debug_crc = _crc16_update(_debug_crc, space_left);
    }

    while(bytes > 31)
    {
        usart_push_block(ptr, 31, false);
        for(uint8_t b = 0; b < 31; ++ b)
            _debug_crc = _crc16_update(_debug_crc, *ptr ++);

        bytes -= 31;
        // FOOTER
        usart_push_block((uint8_t *) &_debug_crc, 2, true);
        _debug_crc = 0xFFFF;

        // HEADER
        uint8_t k = USART_DEBUG;
        usart_push_block(&k, 1, false);
        _debug_crc = _crc16_update(_debug_crc, k);
    }

    if(bytes)
    {
        usart_push_block(ptr, bytes, false);
        while(bytes)
        {
            _debug_crc = _crc16_update(_debug_crc, *ptr ++);
            -- bytes;
            ++ _debug_current_size;
        }
    }

    if(_debug_current_size == 31)
    {
        // FOOTER
        usart_push_block((uint8_t *) &_debug_crc, 2, true);
        _debug_crc = 0xFFFF;
        _debug_current_size = 0;
    }
}

void _debug_commit(void)
{
    if(!_debug_current_size)
        return;

    uint8_t zero = 0;
    while(_debug_current_size < 31)
    {
        usart_push_block(&zero, 1, false);
        _debug_crc = _crc16_update(_debug_crc, 0);
        ++ _debug_current_size;
    }

    // FOOTER
    usart_push_block((uint8_t *) &_debug_crc, 2, true);
    _debug_crc = 0xFFFF;
    _debug_current_size = 0;
}

void _debug_int(uint32_t input, bool sign, uint8_t zero_padding)
{
    char buf[32];
    uint8_t b = 0;
    union
    {
        uint32_t    u;
        int32_t     d;
    } value;

    value.u = input;

    bool negative = false;
    if(sign && value.d < 0)
    {
        negative = true;
        value.d = -value.d;
    }

    do
    {
        buf[b ++] = '0' + value.u % 10;
        value.u /= 10;
    }
    while(value.u);

    while(b < zero_padding)
        buf[b ++] = '0';

    if(negative)
        buf[b ++] = '-';

    while(b)
    {
        -- b;
        _debug_push((uint8_t *) buf + b, 1);
    }
}

void _debug_hex(uint32_t value, uint8_t zero_padding)
{
    char buf[8];
    uint8_t b = 0;

    do
    {
        uint8_t digit = value % 16;
        buf[b ++] = digit < 10 ? '0' + digit : 'a' + digit - 10;;
        value /= 16;
    }
    while(value);

    while(b < zero_padding)
        buf[b ++] = '0';

    while(b)
    {
        -- b;
        _debug_push((uint8_t *) buf + b, 1);
    }
}

void debug_complex(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    uint8_t c;
    while((c = pgm_read_byte(fmt ++)))
    {
        if(c != '%')
        {
            _debug_push(&c, 1);
            continue;
        }

        c = pgm_read_byte(fmt ++);
        bool long_int = false;
        uint8_t zero_padding = 0;
        if(c == '0') // zero padding
        {
            c = pgm_read_byte(fmt ++);
            if(!c)
                break;

            if('0' <= c && c <= '9')
                zero_padding = c - '0';

            c = pgm_read_byte(fmt ++);
        }

        if(c == 'l')
        {
            long_int = true;
            c = pgm_read_byte(fmt ++);
        }

        if(!c)
            break;

        switch(c)
        {
            case 'u':
            case 'd':
                _debug_int(
                    long_int    ? va_arg(args, long unsigned int)
                                : (uint32_t) va_arg(args, unsigned int),
                    (c != 'u'),
                    zero_padding);

                break;

            case 'x':
                _debug_hex(
                    long_int    ? va_arg(args, long unsigned int)
                                : (uint32_t) va_arg(args, unsigned int),
                    zero_padding);
                break;

            case 'c':
                {
                    uint8_t d = va_arg(args, int);
                    _debug_push(&d, 1);
                }

                break;

            case 's':
                {
                    char *ptr = va_arg(args, char *);
                    _debug_push((uint8_t *) ptr, strlen(ptr));
                }
                break;

            default:
                _debug_push(&c, 1);
                break;
        }
    }

    va_end(args);
    _debug_commit();
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
