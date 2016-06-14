#ifndef __PROTOCOL_MESSAGES_DATA_H__
#define __PROTOCOL_MESSAGES_DATA_H__

#include "common.h"

//
// MessageData
//  Wiadomość z danymi (pakiet),
//  rozmiar: 5-253 bajty
//
//  [3]     kind        - typ wiadomości = 0x7
//  [5]     size        - rozmiar = 0-31 (*8 = 0 - 248 bajtów)
//                        liczony w 8 bajtowych blokach
//  [16]    macaddr     - adres MAC nadawcy
//  [16]    dst_macaddr - adres MAC odbiorcy
//  [*]     data[]      - dane
//

#define KIND_DATA 0x7

typedef struct message_data
{
    uint8_t kind    : 3;
    uint8_t size    : 5;

    uint16_t    macaddr;
    uint16_t    dst_macaddr;
    uint8_t     data[];
} MessageData;

typedef const MessageData * const MessageData_cptr;

inline
uint8_t message_data_get_size(MessageData_cptr msg)
{
    return sizeof(MessageData) + msg->size * 8;
}

#ifdef __AVR__
////////////////////////////////////////////////////////////////////////////////

// TODO

#endif // __AVR__
#endif // __PROTOCOL_MESSAGES_DATA_H__
