#ifndef __PROTOCOL_STRUCTS_H__
#define __PROTOCOL_STRUCTS_H__

#include <stdint.h>

#include "common.h"

#ifndef SETTINGS_MESSAGES_ENABLE
#error "Missing messages configuration!"
#endif

enum MessageVariableMasks
{
    MESSAGE_VARIABLE_KIND_MASK      = 0x3F,
    MESSAGE_VARIABLE_SIZE_HIGH_MASK = 0xC0,
};

typedef enum MessageSize
{
    SIZE_CONSTANT   = 0,
    SIZE_VARIABLE   = 1,
    SIZE_MASK       = 1,
} MessageSize;

typedef enum MessageKind
{
#define REGISTER_MESSAGE(NAME, Name, size_type, id)                         \
    KIND_ ## NAME = SIZE_ ## size_type | (id << 1),

    SETTINGS_MESSAGES_ENABLE
#undef REGISTER_MESSAGE
} MessageKind;

typedef struct MessageBase
{
    // NOTICE: keep in sync with _CONSTANT and _VARIABLE

    uint8_t kind;
    // [00000000]
    //  ^ size_type
    //   ^^^^^^^ constant_type
    //   ^^^^^ variable_type
    //        ^^ two bits of variable_size
} Message;

struct MessageBase_CONSTANT
{
    uint8_t kind;
};

struct MessageBase_VARIABLE
{
    uint8_t kind;
    uint8_t size_low;
};

typedef const Message const Message_cptr;
typedef const struct MessageBase_CONSTANT const MessageBase_CONSTANT_cptr;
typedef const struct MessageBase_VARIABLE const MessageBase_VARIABLE_cptr;

#define REGISTER_MESSAGE(NAME, Name, size_type, id)                         \
    typedef struct MessageBase_ ## size_type Message ## Name ## Base;       \
    typedef const struct Message ## Name const Message ## Name ## _cptr;

SETTINGS_MESSAGES_ENABLE
#undef REGISTER_MESSAGE


bool message_is_variable_size(Message_cptr *msg);

void message_set_size(struct MessageBase_VARIABLE *msg, uint16_t size);
uint16_t message_get_size(MessageBase_VARIABLE_cptr *msg);

void message_set_kind(struct MessageBase_VARIABLE *msg, const uint8_t kind);
uint8_t message_get_kind(MessageBase_VARIABLE_cptr *msg);

#define REGISTER_MESSAGE(NAME, Name, size_type, id)                         \
    void put_Message ## Name(Time_cptr *, uint8_t *);

SETTINGS_MESSAGES_ENABLE
#undef REGISTER_MESSAGE

void put_message(uint8_t_cptr *msg, uint16_t bytes_no);

bool message_is_variable_size(Message_cptr *msg)
{
    return (msg->kind & SIZE_MASK) == SIZE_VARIABLE;
}

void message_set_size(struct MessageBase_VARIABLE *msg, uint16_t size)
{
    msg->size_low = size & 0xFF;
    size >>= 2;
    msg->kind   = (msg->kind & MESSAGE_VARIABLE_KIND_MASK)
                | (size & MESSAGE_VARIABLE_SIZE_HIGH_MASK);
}

uint16_t message_get_size(MessageBase_VARIABLE_cptr *msg)
{
    return (    (uint16_t) (msg->kind & MESSAGE_VARIABLE_SIZE_HIGH_MASK) << 8)
            |   msg->size_low;
}

void message_set_kind(struct MessageBase_VARIABLE *msg, const uint8_t kind)
{
    msg->kind   = (kind & MESSAGE_VARIABLE_KIND_MASK)
                | (msg->kind & MESSAGE_VARIABLE_SIZE_HIGH_MASK);
}

uint8_t message_get_kind(MessageBase_VARIABLE_cptr *msg)
{
    return msg->kind & MESSAGE_VARIABLE_KIND_MASK;
}

#endif // __PROTOCOL_STRUCTS_H__
