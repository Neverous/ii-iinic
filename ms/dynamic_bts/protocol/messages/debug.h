#ifndef __PROTOCOL_MESSAGES_DEBUG_H__
#define __PROTOCOL_MESSAGES_DEBUG_H__

#include "common.h"

//
// MessageDebug
//  Wiadomość debugowania,
//  dla zgodności ze sposobem w jaki usart_complex obsługuje stdio
//  rozmiar: 32 bajty
//
//  (USART_DEBUG = 0x23)
//
//  [3]     kind        - typ wiadomości = 0x3
//  [5]     kind_high   - pozostała część typu dla zgodności z USART = 0x4
//  [248]   text[31]    - treść komunikatu (/jego część)
//

#define KIND_DEBUG 0x3

typedef struct message_debug
{
    uint8_t kind        : 3;
    uint8_t kind_high   : 5;

    uint8_t text[31];
} MessageDebug;

typedef const MessageDebug * const MessageDebug_cptr;

inline
uint8_t message_debug_get_size(__unused__ MessageDebug_cptr msg)
{
    return sizeof(MessageDebug);
}

#endif // __PROTOCOL_MESSAGES_DEBUG_H__
