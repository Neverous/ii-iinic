#ifndef __PROTOCOL_MESSAGES_H__
#define __PROTOCOL_MESSAGES_H__

#if __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
#error "This code supports only little endian machines!"
#endif

#include "common.h"

#ifndef __AVR__
#pragma pack(push, 1)
#endif

#include "messages/data.h"
#include "messages/debug.h"
#include "messages/gather.h"
#include "messages/neighbourhood.h"
#include "messages/neighbours.h"
#include "messages/ping.h"
#include "messages/request.h"
#include "messages/response.h"
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

uint8_t message_get_size(Message_cptr msg)
{
    switch(msg->kind)
    {
#define CASE_KIND(NAME, Name, name) \
    case KIND_ ## NAME:             \
        return message_ ## name ## _get_size((Message ## Name ## _cptr) msg)

#define CASE_SUBKIND(NAME, Name, name)  \
    case SUBKIND_ ## NAME:              \
        return message_ ## name ## _get_size((Message ## Name ## _cptr) submsg)

        CASE_KIND(DATA,             Data,               data);
#ifndef __AVR__
    case KIND_DEBUG:
        {
            MessageDebug_cptr submsg = (MessageDebug_cptr) msg;
            switch(submsg->subkind)
            {
                CASE_SUBKIND(   DEBUG_ASSIGNMENT, DebugAssignment,
                                debug_assignment);

                CASE_SUBKIND(   DEBUG_NODE_SPEAK, DebugNodeSpeak,
                                debug_node_speak);

                CASE_SUBKIND(   DEBUG_ROOT_CHANGE, DebugRootChange,
                                debug_root_change);

                CASE_SUBKIND(DEBUG_TEXT, DebugText, debug_text);
            }
        }
        break;
#endif
        CASE_KIND(GATHER,           Gather,             gather);
        CASE_KIND(NEIGHBOURHOOD,    Neighbourhood,      neighbourhood);
        CASE_KIND(NEIGHBOURS,       Neighbours,         neighbours);
#ifdef __AVR__
        CASE_KIND(PING,             Ping,               ping);
#endif
        CASE_KIND(REQUEST,          Request,            request);
        CASE_KIND(RESPONSE,         Response,           response);
        CASE_KIND(SYNCHRONIZATION,  Synchronization,    synchronization);

#undef CASE_KIND
    }

    return 1;
}

uint8_t validate_message(   Message_cptr msg, uint8_t **buffer_ptr,
                            uint8_t_cptr buffer_end)
{
    uint8_t size = message_get_size(msg);
    if(*buffer_ptr + size + 2 > buffer_end)
        return KIND_EOF;

    *buffer_ptr += 1;
    if(crc16((uint8_t_cptr) msg, size + 2))
        return KIND_INVALID;

    *buffer_ptr += size + 1;
    return msg->kind;
}

#endif // __PROTOCOL_STRUCTS_H__
