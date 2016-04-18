#ifndef __PROTOCOL_TYPES_H__
#define __PROTOCOL_TYPES_H__

#include <stdint.h>
#include "../config.h"

#ifndef MESSAGES_CONFIGURATION
#error "Missing messages configuration!"
#endif

typedef enum _MessageSize
{
    SIZE_CONSTANT = 0,
    SIZE_VARIABLE = 1,
} MessageSize;

typedef enum _MessageKind
{
#define REGISTER_MESSAGE(NAME, Name, size_type, id)                         \
    KIND_ ## NAME = SIZE_ ## size_type | (id << 1),

    MESSAGES_CONFIGURATION
#undef REGISTER_MESSAGE
} MessageKind;

typedef struct _MessageBase
{
    // NOTICE: keep in sync with _CONSTANT and _VARIABLE
    //
    uint8_t kind;
    // [00000000]
    //  ^ size_type
    //   ^^^^^^^ constant_type
    //   ^^^^^ variable_type
    //        ^^ two bits of variable_size
} Message;

struct _MessageBase_CONSTANT
{
    uint8_t kind;
};

struct _MessageBase_VARIABLE
{
    uint8_t kind;
    uint8_t size_low;
};

#define REGISTER_MESSAGE(NAME, Name, size_type, id)                         \
    typedef struct _MessageBase_ ## size_type Message ## Name ## _base;

MESSAGES_CONFIGURATION
#undef REGISTER_MESSAGE

void message_set_size(struct _MessageBase_VARIABLE *msg, uint16_t size)
{
    msg->size_low = size & 0xFF;
    size >>= 2;
    msg->kind = (msg->kind & 0x3F) | (size & 0xC0);
}

uint16_t message_get_size(const struct _MessageBase_VARIABLE *msg)
{
    return ((uint16_t) (msg->kind & 0xC0) << 8) | msg->size_low;
}

#define REGISTER_MESSAGE(NAME, Name, size_type, id)                         \
    void put_Message ## Name(Time_cptr *, uint8_t *);

MESSAGES_CONFIGURATION
#undef REGISTER_MESSAGE

#endif // __PROTOCOL_TYPES_H__
