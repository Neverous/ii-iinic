#ifndef __PROTOCOL_MESSAGES_DEBUG_H__
#define __PROTOCOL_MESSAGES_DEBUG_H__

#include "common.h"
#include "neighbours_structs.h"
#include "request_structs.h"

//
// MessageDebugAssignment
//  Wiadomość informująca o aktualnych przydziałach
//  rozmiar: 1 + 6 * SETTINGS_MAX_NODES bajtów
//
//  [3] kind    - typ wiadomości = 0x3 (KIND_DEBUG)
//  [5] subkind - pozostała część typu = 0x0 (SUBKIND_DEBUG_ASSIGNMENT)
//  [*] macaddr[SETTINGS_MAX_NODES]     - adresy MAC
//  [*] assignment[SETTINGS_MAX_NODES]  - przydziały
//

//
// MessageDebugNodeSpeak
//  Wiadomość informująca o tym że nadeszła jakaś wiadomość
//  rozmiar: 4 bajty
//
//  [3]     kind    - typ wiadomości = 0x3 (KIND_DEBUG)
//  [5]     subkind - pozostała część typu = 0x1 (SUBKIND_DEBUG_NODE_SPEAK)
//  [16]    macaddr - adres MAC usłyszanego urządzenia
//  [8]     bytes   - liczba otrzymanych bajtów
//

//
// MessageDebugRootChange
//  Wiadomość informująca o tym że zmienił się korzeń
//  rozmiar: 3 bajty
//
//  [3]     kind            - typ wiadomości = 0x3 (KIND_DEBUG)
//  [5]     subkind         - pozostała część typu = 0x2
//                              (SUBKIND_DEBUG_ROOT_CHANGE)
//  [16]    root_macaddr    - adres MAC korzenia
//

//
// MessageDebugText
//  Wiadomość debugowania,
//  dla zgodności ze sposobem w jaki usart_complex obsługuje stdio
//  rozmiar: 32 bajty
//
//  (USART_DEBUG = 0x23)
//
//  [3]     kind        - typ wiadomości = 0x3
//  [5]     subkind     - pozostała część typu dla zgodności z USART = 0x4
//  [248]   text[31]    - treść komunikatu (/jego część)
//

#define KIND_DEBUG 0x3
#define SUBKIND_DEBUG_ASSIGNMENT    0x00
#define SUBKIND_DEBUG_NODE_SPEAK    0x01
#define SUBKIND_DEBUG_ROOT_CHANGE   0x02
#define SUBKIND_DEBUG_TEXT          0x04

typedef struct message_debug
{
    uint8_t kind    : 3;
    uint8_t subkind : 5;
} MessageDebug;

typedef struct message_debug_assignment
{
    uint8_t kind    : 3;
    uint8_t subkind : 5;

    uint16_t    macaddr[SETTINGS_MAX_NODES];
    Assignment  assignment[SETTINGS_MAX_NODES];
} MessageDebugAssignment;

typedef struct message_debug_node_speak
{
    uint8_t kind    : 3;
    uint8_t subkind : 5;

    uint16_t    macaddr;
    uint8_t     bytes;
} MessageDebugNodeSpeak;

typedef struct message_debug_root_change
{
    uint8_t kind    : 3;
    uint8_t subkind : 5;

    uint16_t root_macaddr;
} MessageDebugRootChange;

typedef struct message_debug_text
{
    uint8_t kind    : 3;
    uint8_t subkind : 5;

    uint8_t text[31];
} MessageDebugText;

typedef const MessageDebug * const MessageDebug_cptr;
typedef const MessageDebugAssignment * const MessageDebugAssignment_cptr;
typedef const MessageDebugNodeSpeak * const MessageDebugNodeSpeak_cptr;
typedef const MessageDebugRootChange * const MessageDebugRootChange_cptr;
typedef const MessageDebugText * const MessageDebugText_cptr;

uint8_t message_debug_assignment_get_size(
    __unused__ MessageDebugAssignment_cptr msg)
{
    return sizeof(MessageDebugAssignment);
}

uint8_t message_debug_node_speak_get_size(
    __unused__ MessageDebugNodeSpeak_cptr msg)
{
    return sizeof(MessageDebugNodeSpeak);
}

uint8_t message_debug_root_change_get_size(
    __unused__ MessageDebugRootChange_cptr msg)
{
    return sizeof(MessageDebugRootChange);
}

uint8_t message_debug_text_get_size(__unused__ MessageDebugText_cptr msg)
{
    return sizeof(MessageDebugText);
}

uint8_t message_debug_get_size(MessageDebug_cptr msg)
{
    switch(msg->subkind)
    {
        case SUBKIND_DEBUG_ASSIGNMENT:
            return message_debug_assignment_get_size(
                (MessageDebugAssignment_cptr) msg);

        case SUBKIND_DEBUG_NODE_SPEAK:
            return message_debug_node_speak_get_size(
                (MessageDebugNodeSpeak_cptr) msg);

        case SUBKIND_DEBUG_ROOT_CHANGE:
            return message_debug_root_change_get_size(
                (MessageDebugRootChange_cptr) msg);

        case SUBKIND_DEBUG_TEXT:
            return message_debug_text_get_size(
                (MessageDebugText_cptr) msg);
    }

    return 1;
}

#if defined(__AVR__) && defined(__USART_COMPLEX__)
////////////////////////////////////////////////////////////////////////////////

void put_debug_assignment_message(void);
void put_debug_node_speak_message(uint16_t macaddr, uint8_t bytes);
void put_debug_root_change_message(uint16_t root_macaddr);


////////////////////////////////////////////////////////////////////////////////

void put_debug_assignment_message(void)
{
    uint16_t crc = 0xFFFF;
    {
        MessageDebug msg;
        msg.kind       = KIND_DEBUG;
        msg.subkind    = SUBKIND_DEBUG_ASSIGNMENT;
        usart_push_block((uint8_t *) &msg, 1, false);
        crc = _crc16_update(crc, *(uint8_t *) &msg);
    }

    for(uint8_t n = 0; n < SETTINGS_MAX_NODES; ++ n)
    {
        usart_push_block((uint8_t *) &neighbours.node[n].macaddr, 2, false);
        crc = _crc16_update(crc, neighbours.node[n].macaddr & 0xFF);
        crc = _crc16_update(crc, neighbours.node[n].macaddr >> 8);
    }

    uint8_t *ptr = (uint8_t *) request.assignment;
    usart_push_block(ptr, sizeof(Assignment) * SETTINGS_MAX_NODES, false);

    uint8_t_cptr end_ptr = ptr + sizeof(Assignment) * SETTINGS_MAX_NODES;
    while(ptr < end_ptr)
        crc = _crc16_update(crc, *ptr ++);

    usart_push_block((uint8_t *) &crc, 2, true);

    DEBUG(TIME_NULL "|U|-DEBUG_ASSIGNMENT\r\n");
}

void put_debug_node_speak_message(uint16_t macaddr, uint8_t bytes)
{
    uint16_t crc = 0xFFFF;
    {
        MessageDebug msg;
        msg.kind       = KIND_DEBUG;
        msg.subkind    = SUBKIND_DEBUG_NODE_SPEAK;
        usart_push_block((uint8_t *) &msg, 1, false);
        crc = _crc16_update(crc, *(uint8_t *) &msg);
    }

    usart_push_block((uint8_t *) &macaddr, 2, false);
    crc = _crc16_update(crc, macaddr & 0xFF);
    crc = _crc16_update(crc, macaddr >> 8);

    usart_push_block(&bytes, 1, false);
    crc = _crc16_update(crc, bytes);

    usart_push_block((uint8_t *) &crc, 2, true);
    DEBUG(TIME_NULL "|U|-DEBUG_NSPEAK(0x%04x,%u)\r\n", macaddr, bytes);
}

void put_debug_root_change_message(uint16_t root_macaddr)
{
    uint16_t crc = 0xFFFF;
    {
        MessageDebug msg;
        msg.kind       = KIND_DEBUG;
        msg.subkind    = SUBKIND_DEBUG_ROOT_CHANGE;
        usart_push_block((uint8_t *) &msg, 1, false);
        crc = _crc16_update(crc, *(uint8_t *) &msg);
    }

    usart_push_block((uint8_t *) &root_macaddr, 2, false);
    crc = _crc16_update(crc, root_macaddr & 0xFF);
    crc = _crc16_update(crc, root_macaddr >> 8);

    usart_push_block((uint8_t *) &crc, 2, true);
    DEBUG(TIME_NULL "|U|-DEBUG_ROOT_CHANGE(0x%04x)\r\n", root_macaddr);
}

#endif // __AVR__ && __USART_COMPLEX__
#endif // __PROTOCOL_MESSAGES_DEBUG_H__
