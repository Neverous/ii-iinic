#ifndef __PROTOCOL_USART_H__
#define __PROTOCOL_USART_H__

#include "messages.h"

#ifdef __USART_COMPLEX__
inline
uint8_t usart_get_packet_size(uint8_t kind)
{
    return message_get_size((Message_cptr) &kind) + 2;
}

inline
void handle_usart_message(Time_cptr time, Message_cptr msg)
{
    switch(msg->kind)
    {
        case KIND_PING:
            handle_usart_ping(time, (MessagePing_cptr) msg);
            break;
    }
}

inline
void handle_usart(void)
{
    Time time; time_get_now(&time);
    DEBUG(  TIME_FMT "|U|+%ub\r\n",
            TIME_FMT_DATA(time), usart_pending_read());

    uint8_t count = 0;
    while(true)
    {
        Message_cptr msg;
        if(usart_peek((const uint8_t **) &msg, 1) < 1)
            break;

        uint8_t size = message_get_size(msg);
        if(usart_pending_read() < size + 2)
            break;

        if(usart_crc16(size + 2))
        {
            usart_pop(1);
            continue;
        }

        uint8_t peeked = usart_peek((const uint8_t **) &msg, size + 2);

        if(peeked == size + 2)
        {
            handle_usart_message(&time, msg);
            ++ count;
            usart_pop(peeked);
            continue;
        }

        else
        {
            usart_pop(size + 2);
            continue;
        }
    }

    DEBUG(TIME_FMT "|U|+%um\r\n", TIME_FMT_DATA(time), count);
}

#endif // __USART_COMPLEX__

#endif // __PROTOCOL_USART_H__
