#ifndef __USART_STRUCTS__
#define __USART_STRUCTS__

struct USARTData
{
    IOBuffer    out;
    IOBuffer    in;
};

extern struct USARTData usart;

enum USART_PACKET_KIND
{
    USART_DEBUG = 0x23,
};

typedef struct usart_packet
{
    uint8_t kind;
} USARTPacket;

typedef struct usart_debug
{
    USARTPacket base;
    uint8_t debug[31];
} USARTDebug;

uint8_t usart_get_builtin_packet_size(const uint8_t kind);


inline
uint8_t usart_get_builtin_packet_size(const uint8_t kind)
{
    switch(kind)
    {
        case USART_DEBUG:
            return sizeof(USARTDebug);
    }

    return 1;
}

#endif // __USART_STRUCTS__
