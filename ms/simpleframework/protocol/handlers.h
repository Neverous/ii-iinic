#ifndef __PROTOCOL_HANDLERS_H__
#define __PROTOCOL_HANDLERS_H__

#include <stdint.h>
#include "../config.h"
#include "types.h"
#include "../crc.h"

#ifndef MESSAGES_CONFIGURATION
#error "Missing messages configuration!"
#endif

uint8_t handle_messages(iinic_timing_cptr *time, const uint16_t rssi, uint8_t *buffer_ptr, const uint8_t const *buffer_end, const uint8_t options)
{
    debug("[%lu] read %d bytes\r\n", *(uint32_t *) time, buffer_end - buffer_ptr);
    while(buffer_ptr < buffer_end)
    {
        Message *msg = (Message *) buffer_ptr;

        buffer_ptr += 2;
        // Validate message size
        switch(msg->kind)
        {
#define REGISTER_MESSAGE(NAME, Name, size_type, id)                         \
    case KIND_ ## NAME:                                                     \
        if(SIZE_ ## size_type == SIZE_CONSTANT)                             \
            buffer_ptr += sizeof(struct Message ## Name);                   \
        else                                                                \
            buffer_ptr += message_get_size((struct _MessageBase_VARIABLE *) msg); \
                                                                            \
        break;

MESSAGES_CONFIGURATION
#undef REGISTER_MESSAGE
            default:
                debug("[%lu] dropping messages: unknown kind %u\r\n", *(uint32_t *) time, msg->kind);
                return 1;
        }

        if(buffer_ptr > buffer_end)
        {
            debug("[%lu] dropping messages: buffer overflow\r\n", *(uint32_t *) time);
            return 1;
        }

        // Validate CRC
        if(crc16((uint8_t *) msg, buffer_ptr - (uint8_t *) msg))
        {
            debug("[%lu] dropping message: invalid crc\r\n", *(uint32_t *) time);
            continue;
        }

        // Handle message
        switch(msg->kind)
        {
#define REGISTER_MESSAGE(NAME, Name, size_type, id)                         \
    case KIND_ ## NAME:                                                     \
        handle_Message ## Name(time, rssi, (struct Message ## Name *) msg, options); \
        break;

MESSAGES_CONFIGURATION
#undef REGISTER_MESSAGE
        }

        break;
    }

    return 0;
}

#define REGISTER_MESSAGE(NAME, Name, size_type, id)                         \
    void put_Message ## Name(iinic_timing_cptr *time, uint8_t *ctx) \
    {                                                                       \
        uint8_t *txbuffer_end = txbuffer + SETTINGS_TXBUFFER_SIZE;          \
        uint8_t *message_end = write_Message ## Name(time, txbuffer_ptr, txbuffer_end, ctx); \
        if(!message_end)                                                    \
            return;                                                         \
                                                                            \
        if(message_end + 2 > txbuffer_end)                                  \
            return;                                                         \
                                                                            \
        *(uint16_t *) message_end = crc16(txbuffer_ptr, message_end - txbuffer_ptr); \
        txbuffer_ptr = message_end + 2;                                     \
    }

MESSAGES_CONFIGURATION

#undef REGISTER_MESSAGE

void on_init(iinic_timing_cptr *time, const uint8_t options)
{
#define REGISTER_MESSAGE(NAME, Name, size_type, id)                         \
    on_init_Message ## Name(time, options);

MESSAGES_CONFIGURATION
#undef REGISTER_MESSAGE
}

void on_frame_start(iinic_timing_cptr *frame_start, iinic_timing *frame_deadline, const uint8_t options)
{
#define REGISTER_MESSAGE(NAME, Name, size_type, id)                         \
    on_frame_start_Message ## Name(frame_start, frame_deadline, options);

MESSAGES_CONFIGURATION
#undef REGISTER_MESSAGE
}

void on_slot_start(iinic_timing_cptr *slot_start, iinic_timing *slot_deadline, const uint8_t options)
{
#define REGISTER_MESSAGE(NAME, Name, size_type, id)                         \
    on_slot_start_Message ## Name(slot_start, slot_deadline, options);

MESSAGES_CONFIGURATION
#undef REGISTER_MESSAGE
}

void on_slot_end(iinic_timing_cptr *slot_end, const uint8_t options)
{
    txbuffer_ptr = txbuffer;

#define REGISTER_MESSAGE(NAME, Name, size_type, id)                         \
    on_slot_end_Message ## Name(slot_end, options);

MESSAGES_CONFIGURATION
#undef REGISTER_MESSAGE
}

void on_frame_end(iinic_timing_cptr *frame_end, uint8_t options)
{
#define REGISTER_MESSAGE(NAME, Name, size_type, id)                         \
    on_frame_end_Message ## Name(frame_end, options);

MESSAGES_CONFIGURATION
#undef REGISTER_MESSAGE
}

#endif // __PROTOCOL_HANDLERS_H__
