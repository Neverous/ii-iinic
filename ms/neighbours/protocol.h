#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <stdint.h>

#include "crc.h"

struct packet_header
{
    uint16_t    len;
}; // struct packet_header

struct packet_footer
{
    uint16_t crc;
}; // struct packet_footer

struct msg
{
    uint8_t kind;
}; // struct msg

const uint8_t MSG_HELLO = 1;
struct msg_hello
{
    struct msg msg;
    uint16_t macaddr;
}; // struct msg_hello

inline
static uint16_t __packet_size(uint16_t data_size)
{
    return sizeof(struct packet_header) + data_size + sizeof(struct packet_footer);
}

inline
static uint8_t *__packet_header(uint8_t *buff, uint16_t data_size)
{
    struct packet_header *phd = (struct packet_header *) buff;
    phd->len = __packet_size(data_size);
    return (uint8_t *) (phd + 1);
}

inline
static uint8_t *__packet_footer(uint8_t *buff, uint16_t data_size)
{
    const uint16_t header_data_size = sizeof(struct packet_header) + data_size;
    struct packet_footer *ptl = (struct packet_footer *) (buff + header_data_size);
    ptl->crc = crc16(buff, header_data_size);
    return (uint8_t *) (ptl + 1);
}

inline
static uint8_t *hello_msg_write(uint8_t *buff, uint8_t *buff_end, uint16_t macaddr)
{
    if(buff + __packet_size(sizeof(struct msg_hello)) > buff_end)
        return 0;

    struct msg_hello *msg = (struct msg_hello *) __packet_header(buff, sizeof(struct msg_hello));
    msg->msg.kind   = MSG_HELLO;
    msg->macaddr    = macaddr;

    return __packet_footer(buff, sizeof(struct msg_hello));
}

inline
static uint8_t *hello_msg_read(uint8_t *buff, uint8_t *buff_end, uint16_t *macaddr)
{
    if(buff + __packet_size(sizeof(struct msg_hello)) > buff_end)
        return 0;

    if(crc16(buff, __packet_size(sizeof(struct msg_hello))))
        return 0;

    struct packet_header *phd   = (struct packet_header *) buff;
    struct msg_hello *msg       = (struct msg_hello *) (phd + 1);
    struct packet_footer *ptl   = (struct packet_footer *) (msg + 1);

    if(phd->len != __packet_size(sizeof(struct msg_hello)))
        return 0;

    if(msg->msg.kind != MSG_HELLO)
        return 0;

    *macaddr = msg->macaddr;
    return (uint8_t *) (ptl + 1);
}

#endif // __PROTOCOL_H__
