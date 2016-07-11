#ifndef __PROTOCOL_MESSAGES_PING_H__
#define __PROTOCOL_MESSAGES_PING_H__

#include "common.h"

//
// MessagePing
//  Wiadomość informująca urządzenie że jest podłączone do komputera,
//  pozwala przerzucić część obliczeń z urządzenia. Taki sam typ (0x3)
//  jak Debug w celu oszczędzania identyfikatorów wiadomości, a ping jest
//  wysyłany tylko po USART od komputera do urządzenia.
//
//  Tryby:
//      MODE_HIDDEN         - komputer odłączony, jedyne wiadomości to debug
//      MODE_MONITOR        - komputer podłączony w trybie monitorowania,
//                            przekazywanie niektórych wiadomości do komputera
//                            w celu łatwiejszego monitorowania sieci
//
//  Opcje:
//      OPTION_MASTER       - urządzenie zeruje pierwszy bajt
//                            adresu mac aby wymusić bycie korzeniem
//
//  rozmiar: 1 bajt
//
//  [3] kind            - typ wiadomości = 0x3 (=KIND_DEBUG)
//  [5] mode | options  - tryb pracy
//

#define KIND_PING 0x3

enum Ping
{
    P_MODE_HIDDEN          = 0,
    P_MODE_MONITOR         = 1,
    P_MODE_MASK            = 1,

    P_OPTION_MASTER        = 2
};

typedef struct message_ping
{
    uint8_t kind    : 3;
    uint8_t options : 5;
} MessagePing;

typedef const MessagePing * const MessagePing_cptr;

uint8_t message_ping_get_size(__unused__ MessagePing_cptr msg)
{
    return sizeof(MessagePing);
}


#ifdef __AVR__
////////////////////////////////////////////////////////////////////////////////

struct PingData
{
    uint8_t mode;
    uint8_t ttl;
};

extern struct PingData ping;


////////////////////////////////////////////////////////////////////////////////

void handle_usart_ping(Time_cptr time, MessagePing_cptr msg);
void validate_ping(void);


////////////////////////////////////////////////////////////////////////////////

void handle_usart_ping(Time_cptr time, MessagePing_cptr msg)
{
    DEBUG(  TIME_FMT "|U|+PING(0x%02x)\r\n",
            TIME_FMT_DATA(*time), msg->options);

    ping.mode = msg->options & P_MODE_MASK;
    ping.ttl = UINT8_MAX;

    if(msg->options & P_OPTION_MASTER)
    {
        if(device_macaddr & 0xFF00)
        {
            device_macaddr &= 0xFF;
            INFO(   TIME_FMT "| |MAC=0x%04x\r\n",
                    TIME_FMT_DATA(*time), device_macaddr);
        }
    }

    else
    {
        if(!(device_macaddr & 0xFF00))
        {
            device_macaddr = iinic_mac;
            INFO(   TIME_FMT "| |MAC=0x%04x\r\n",
                    TIME_FMT_DATA(*time), device_macaddr);
        }
    }
}

void validate_ping(void)
{
    if(!ping.ttl)
        return;

    -- ping.ttl;
    if(!ping.ttl)
    {
        ping.mode = P_MODE_HIDDEN;
        device_macaddr = iinic_mac;
        WARNING(TIME_NULL "| |PING_TIMEOUT\r\n", 0);
    }
}

#endif // __AVR__
#endif // __PROTOCOL_MESSAGES_PING_H__
