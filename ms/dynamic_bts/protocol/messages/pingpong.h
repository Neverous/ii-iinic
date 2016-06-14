#ifndef __PROTOCOL_MESSAGES_PINGPONG_H__
#define __PROTOCOL_MESSAGES_PINGPONG_H__

#include "common.h"

//
// MessagePingPong
//  Wiadomość informująca urządzenie że jest podłączone do komputera,
//  pozwala przerzucić część obliczeń z urządzenia
//
//  Tryby:
//      MODE_HIDDEN         - komputer odłączony, jedyne wiadomości to debug
//      MODE_MONITOR        - komputer podłączony w trybie monitorowania,
//                            przekazywanie niektórych wiadomości do komputera
//                            w celu łatwiejszego monitorowania sieci
//      MODE_MONITOR_MASTER - MODE_MONITOR + urządzenie zeruje pierwszy bajt
//                            adresu mac aby wymusić bycie korzeniem
//      MODE_MASTER         - przekazywanie wszystkich wiadomości do komputera
//                            tak jakby był kolejnym urządzeniem podłączonym
//                            drogą radiową (w zamierzeniu wtedy komputer ma
//                            zajmować się przydziałem slotów itd.)
//
//  rozmiar: 1 bajt
//
//  [3] kind    - typ wiadomości = 0x0
//  [5] mode    - tryb pracy
//

#define KIND_PINGPONG 0x0

enum PingPong
{
    PP_PING = 0,
    PP_PONG = 1,

    PP_MODE_HIDDEN          = 0,
    PP_MODE_MONITOR         = 2,
    PP_MODE_MONITOR_MASTER  = 4,
    PP_MODE_MASTER          = 6,
    PP_MODE_MASK            = 6,
};

typedef struct message_pingpong
{
    uint8_t kind    : 3;
    uint8_t options : 5;
} MessagePingPong;

typedef const MessagePingPong * const MessagePingPong_cptr;

uint8_t message_pingpong_get_size(__unused__ MessagePingPong_cptr msg)
{
    return sizeof(MessagePingPong);
}


#ifdef __AVR__
////////////////////////////////////////////////////////////////////////////////

struct PingPongData
{
    uint8_t mode;
    uint8_t ttl;
};

extern struct PingPongData pingpong;


////////////////////////////////////////////////////////////////////////////////

void handle_usart_pingpong(Time_cptr time, MessagePingPong_cptr msg);
void validate_pingpong(void);


////////////////////////////////////////////////////////////////////////////////

void handle_usart_pingpong(Time_cptr time, MessagePingPong_cptr msg)
{
    DEBUG(  TIME_FMT "|U|+PINGPONG(0x%02x)\r\n",
            TIME_FMT_DATA(*time), msg->options);

    if(msg->options & PP_PONG)
        return;

    pingpong.mode = msg->options & PP_MODE_MASK;
    pingpong.ttl = UINT8_MAX;
}

void validate_pingpong(void)
{
    if(!pingpong.ttl)
        return;

    -- pingpong.ttl;
    if(!pingpong.ttl)
    {
        pingpong.mode = PP_MODE_HIDDEN;
        DEBUG(  TIME_FMT "| |PP_TIMEOUT\r\n", (uint16_t) 0, (uint32_t) 0);
    }
}

#endif // __AVR__
#endif // __PROTOCOL_MESSAGES_PINGPONG_H__
