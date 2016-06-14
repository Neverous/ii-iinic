#ifndef __PROTOCOL_MESSAGES_H__
#define __PROTOCOL_MESSAGES_H__

#include "common.h"

#ifndef __AVR__
#pragma pack(push, 1)
#endif

#include "messages/debug.h"
#include "messages/neighbourhood.h"
#include "messages/pingpong.h"
#include "messages/synchronization.h"

#define KIND_EOF        0xE
#define KIND_INVALID    0xF

typedef struct Message
{
    uint8_t kind    : 3;
    uint8_t unused  : 5;
} Message;

#ifndef __AVR__
#pragma pack(pop)
#endif

typedef const Message * const Message_cptr;

inline
uint8_t message_get_size(Message_cptr msg)
{
    switch(msg->kind)
    {
#define CASE_KIND(NAME, Name, name) \
    case KIND_ ## NAME:             \
        return message_## name ## _get_size((Message ## Name ## _cptr) msg)

        CASE_KIND(DEBUG,            Debug,              debug);
        CASE_KIND(NEIGHBOURHOOD,    Neighbourhood,      neighbourhood);
        CASE_KIND(PINGPONG,         PingPong,           pingpong);
        CASE_KIND(SYNCHRONIZATION,  Synchronization,    synchronization);

#undef CASE_KIND
    }

    return 1;
}

inline
uint8_t validate_message(   Message_cptr msg, uint8_t **buffer_ptr,
                            uint8_t_cptr buffer_end)
{
    uint8_t size = message_get_size(msg);
    if(*buffer_ptr + size + 2 > buffer_end)
        return KIND_EOF;

    *buffer_ptr += 1;
    if(crc16((uint8_t_cptr) msg, *buffer_ptr - (uint8_t_cptr) msg + size + 1))
        return KIND_INVALID;

    *buffer_ptr += size + 1;
    return msg->kind;
}

#endif // __PROTOCOL_STRUCTS_H__
