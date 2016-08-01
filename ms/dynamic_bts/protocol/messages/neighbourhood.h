#ifndef __PROTOCOL_MESSAGES_NEIGHBOURHOOD_H__
#define __PROTOCOL_MESSAGES_NEIGHBOURHOOD_H__

#include "common.h"

//
// MessageNeighbourhood
//  Wiadomość służąca do wyświetlania sąsiadów
//  po wciśnięciu przycisku USER na urządzeniu, jego sąsiadom zapali się
//  druga lampka (zależnie od trybu korzeń/klient może to być czerwona bądź
//  zielona). Czas działania można określać poprzez stałą
//      SETTINGS_NEIGHBOURHOOD_TTL w config.h
//  rozmiar: 1 bajt
//
//  [3] kind    - typ wiadomości = 0x2
//  [5] unused  - nieużywana część (dopełnienie do bajtu)
//

#define KIND_NEIGHBOURHOOD 0x2

typedef struct message_neighbourhood
{
    uint8_t kind    : 3;
    uint8_t unused  : 5;
} MessageNeighbourhood;

typedef const MessageNeighbourhood * const MessageNeighbourhood_cptr;

inline
uint8_t message_neighbourhood_get_size(__unused__ MessageNeighbourhood_cptr msg)
{
    return sizeof(MessageNeighbourhood);
}


#ifdef __AVR__
////////////////////////////////////////////////////////////////////////////////

struct NeighbourhoodData
{
    uint8_t ttl;
};

extern struct NeighbourhoodData neighbourhood;


////////////////////////////////////////////////////////////////////////////////

void handle_neighbourhood(  Time_cptr time, MessageNeighbourhood_cptr msg,
                            const uint8_t rssi);

void put_neighbourhood_message(void);
void validate_neighbourhood(void);


////////////////////////////////////////////////////////////////////////////////

void handle_neighbourhood(  Time_cptr time,
                            __unused__ MessageNeighbourhood_cptr msg,
                            const uint8_t rssi)
{
    DEBUG(TIME_FMT "|R|+NHOOD(%u)\r\n", TIME_FMT_DATA(*time), rssi);
    neighbourhood.ttl = SETTINGS_NEIGHBOURHOOD_TTL;
}

void put_neighbourhood_message(void)
{
    uint8_t size = sizeof(MessageNeighbourhood);
    MessageNeighbourhood *msg =
        (MessageNeighbourhood *) control_txbuffer_get(size);

    if(!msg)
        return;

    msg->kind           = KIND_NEIGHBOURHOOD;
    msg->unused         = 0x15; // 0b10101

    control_txbuffer_commit(size);

    Time local_time; time_get_now(&local_time);
    DEBUG(TIME_FMT "|R|-NHOOD\r\n", TIME_FMT_DATA(local_time));
}

inline
void validate_neighbourhood(void)
{
    if(neighbourhood.ttl)
        -- neighbourhood.ttl;
}

#endif // __AVR__
#endif // __PROTOCOL_MESSAGES_NEIGHBOURHOOD_H__
