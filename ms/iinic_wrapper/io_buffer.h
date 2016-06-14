#ifndef __IO_BUFFER_H__
#define __IO_BUFFER_H__

#include <stdint.h>
#include <stdlib.h>

#include "iinic_wrapper.h"

typedef struct io_buffer
{
    uint8_t head;
    volatile uint8_t cap;
    uint8_t tail;
    uint8_t data[256];
} IOBuffer;

uint8_t io_buffer_full(const IOBuffer * const buf);
uint8_t io_buffer_free_size(const IOBuffer * const buf);
uint8_t io_buffer_used_size(const IOBuffer * const buf);
uint8_t io_buffer_push_one(IOBuffer * const buf, const uint8_t byte);
uint8_t io_buffer_push( IOBuffer * const buf,
                        const uint8_t *ptr, const uint8_t bytes);

void io_buffer_commit(IOBuffer * const buf);
uint16_t io_buffer_crc16(IOBuffer * const buf, uint8_t len);
uint8_t io_buffer_pop_one(IOBuffer * const buf);
uint8_t io_buffer_pop(  IOBuffer * const buf, const uint8_t bytes);

uint8_t io_buffer_get_one(IOBuffer * const buf, uint8_t * const byte);
uint8_t io_buffer_get(  IOBuffer * const buf,
                        uint8_t *ptr, const uint8_t bytes);

uint8_t io_buffer_peek_one( const IOBuffer * const buf,
                            const uint8_t **ptr);
uint8_t io_buffer_peek( const IOBuffer * const buf,
                        const uint8_t bytes, const uint8_t **ptr);

uint8_t io_buffer_exchange( IOBuffer *output, IOBuffer * const input,
                            const uint8_t bytes);


uint8_t io_buffer_full(const IOBuffer * const buf)
{
    return buf->head == (uint8_t) (buf->tail + 1);
}

uint8_t io_buffer_free_size(const IOBuffer * const buf)
{
    return 255 - io_buffer_used_size(buf);
}

uint8_t io_buffer_used_size(const IOBuffer * const buf)
{
    return buf->tail - buf->head;
}

uint8_t io_buffer_push_one(IOBuffer * const buf, const uint8_t byte)
{
    if(io_buffer_full(buf))
        return 0;

    buf->data[buf->tail ++] = byte;
    return 1;
}

uint8_t io_buffer_push( IOBuffer * const buf,
                        const uint8_t *ptr, const uint8_t bytes)
{
    uint8_t put = 0;
    while(put < bytes && io_buffer_push_one(buf, *ptr))
        ++ put, ++ ptr;

    return put;
}

void io_buffer_commit(IOBuffer * const buf)
{
    buf->cap = buf->tail;
}

uint16_t io_buffer_crc16(IOBuffer * const buf, uint8_t len)
{
    uint16_t crc = 0xFFFF;
    for(uint8_t p = buf->head; len && p != buf->cap; ++ p)
    {
        crc = _crc16_update(crc, buf->data[p]);
        -- len;
    }

    return crc;
}

uint8_t io_buffer_pop_one(IOBuffer * const buf)
{
    if(buf->head == buf->cap)
        return 0;

    ++ buf->head;
    return 1;
}

uint8_t io_buffer_pop(IOBuffer * const buf, const uint8_t bytes)
{
    uint8_t pop = 0;
    while(pop < bytes && io_buffer_pop_one(buf))
        ++ pop;

    return pop;
}

uint8_t io_buffer_get_one(IOBuffer * const buf, uint8_t * const byte)
{
    if(buf->head == buf->cap)
        return 0;

    *byte = buf->data[buf->head ++];
    return 1;
}

uint8_t io_buffer_get(  IOBuffer * const buf,
                        uint8_t *ptr, const uint8_t bytes)
{
    uint8_t got = 0;
    while(got < bytes && io_buffer_get_one(buf, ptr))
        ++ got, ++ ptr;

    return got;
}

uint8_t io_buffer_peek_one( const IOBuffer * const buf,
                            const uint8_t **ptr)
{
    *ptr = buf->data + buf->head;
    return buf->cap != buf->head;
}

uint8_t io_buffer_peek( const IOBuffer * const buf,
                        const uint8_t bytes, const uint8_t **ptr)
{
    *ptr = buf->data + buf->head;
    if(bytes < (uint8_t) buf->cap - buf->head)
        return bytes;

    return buf->cap - buf->head;
}

uint8_t io_buffer_exchange( IOBuffer *output, IOBuffer * const input,
                            const uint8_t bytes)
{
    uint8_t exchanged = 0;
    while(  exchanged < bytes
        &&  !io_buffer_full(output)
        &&  io_buffer_get_one(input, output->data + output->tail))
        ++ exchanged, ++ output->tail;

    return exchanged;
}

#endif // __IO_BUFFER_H__
