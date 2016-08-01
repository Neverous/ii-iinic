#ifndef __PROTOCOL_BTS_H__
#define __PROTOCOL_BTS_H__

#include <avr/pgmspace.h>

#include "blinker.h"
#include "messages.h"
#include "usart.h"

void control_step(Time_cptr deadline);
void data_step(Time_cptr deadline);

inline
static
void bts_loop(void)
{
    iinic_idle();
    iinic_set_buffer(rxbuffer, SETTINGS_RXBUFFER_SIZE);
    iinic_rx();

    Time now; time_get_now(&now);
    INFO("\r\n" TIME_FMT "|M|BTS\r\n", TIME_FMT_DATA(now));

    while(true)
    {
        Time global_time; time_get_global_now(&global_time);
        blink(&global_time);
        time_align(&global_time,    SETTINGS_CONTROL_FRAME_TIME +
                                    SETTINGS_DATA_FRAME_TIME);

        Time frame_deadline;
        time_global_to_local(&frame_deadline, &global_time);
        Time control_deadline = frame_deadline;
        time_add_i32(&control_deadline, -SETTINGS_DATA_FRAME_TIME);

        control_step(&control_deadline);
        data_step(&frame_deadline);

        validate_neighbourhood();
        validate_neighbours();
#ifdef __USART_COMPLEX__
        validate_ping();
#endif
        validate_request();
        validate_synchronization();
        ++ timer;
    }
}

inline
void control_handle_messages(
    Time_cptr time, const uint8_t rssi,
    uint8_t *buffer_ptr, uint8_t_cptr buffer_end)
{
    DEBUG(  TIME_FMT "|R|+%ub\r\n",
            TIME_FMT_DATA(*time), buffer_end - buffer_ptr);

    uint8_t count = 0;
    while(buffer_ptr < buffer_end)
    {
        Message_cptr msg = (Message_cptr) buffer_ptr;
        switch(validate_message(msg, &buffer_ptr, buffer_end))
        {
            case KIND_GATHER:
                handle_gather(time, (MessageGather_cptr) msg, rssi);
                ++ count;
                break;

            case KIND_NEIGHBOURHOOD:
                handle_neighbourhood(
                    time, (MessageNeighbourhood_cptr) msg, rssi);

                ++ count;
                break;

            case KIND_NEIGHBOURS:
                handle_neighbours(time, (MessageNeighbours_cptr) msg, rssi);
                ++ count;
                break;

            case KIND_REQUEST:
                handle_request(time, (MessageRequest_cptr) msg, rssi);
                ++ count;
                break;

            case KIND_RESPONSE:
                handle_response(time, (MessageResponse_cptr) msg, rssi);
                ++ count;
                break;

            case KIND_SYNCHRONIZATION:
                handle_synchronization(
                    time, (MessageSynchronization_cptr) msg, rssi);

                ++ count;
                break;

            case KIND_EOF:
                ++ buffer_ptr;
                break;
        }
    }

    NOTICE(TIME_FMT "|R|+%um\r\n", TIME_FMT_DATA(*time), count);
}

void control_listen_until(Time_cptr deadline)
{
    uint8_t signal;
    while((signal = timed_poll(IINIC_RX_COMPLETE | IINIC_SIGNAL, deadline)))
    {
        if(signal & IINIC_RX_COMPLETE)
        {
            control_handle_messages(
                (Time_cptr) &iinic_rx_timing,
                _scale_rssi(iinic_rx_rssi),
                rxbuffer, iinic_buffer_ptr);

            iinic_rx();
        }

#ifdef __USART_COMPLEX__
        if(signal & IINIC_SIGNAL)
            handle_usart();
#endif
    }
}

inline
bool control_speak_until(Time_cptr deadline)
{
    if( (   synchronization.valid ||
#ifndef STATIC_ROOT
            synchronization.root.macaddr == device_macaddr) &&
#else
            STATIC_ROOT == device_macaddr) &&
#endif
        (   synchronization.trigger ||
            !((timer + (uint16_t) &synchronization) %
                SETTINGS_SYNCHRONIZATION_PERIOD)))
    {
        put_synchronization_message();
    }

    if(control_txbuffer_ptr == control_txbuffer)
        return true;

    iinic_idle();
    iinic_set_buffer(control_txbuffer, control_txbuffer_ptr - control_txbuffer);
    iinic_tx();

    bool time_left = true;
    if(!timed_poll(IINIC_TX_COMPLETE, deadline))
    {
        time_left = false;
        iinic_idle();
    }

    else
    {
        DEBUG(  TIME_FMT "|R|-%ub\r\n",
                TIME_FMT_DATA(*deadline),
                control_txbuffer_ptr - control_txbuffer);
    }

    iinic_set_buffer(rxbuffer, SETTINGS_RXBUFFER_SIZE);
    iinic_rx();
    control_txbuffer_ptr = control_txbuffer;
    return time_left;
}

inline
void control_step(Time_cptr deadline)
{
    Time random_slot = *deadline;
    time_add_i32(&random_slot, -(random() % SETTINGS_CONTROL_FRAME_TIME));

    if(     iinic_read_button()
        &&  neighbourhood.ttl < SETTINGS_NEIGHBOURHOOD_TTL / 2)
    {
        neighbourhood.ttl = SETTINGS_NEIGHBOURHOOD_TTL;
        put_neighbourhood_message();
    }

    if(!((timer + (uint16_t) &neighbours) % SETTINGS_NEIGHBOURS_PERIOD))
    {
        put_neighbours_message();
    }

#ifndef STATIC_ROOT
    if(synchronization.root.macaddr != device_macaddr &&
#else
    if(STATIC_ROOT != device_macaddr &&
#endif
        !request.assignment[0].ttl &&
        request.bytes_left > 8)
    {
        put_request_message();
    }

    if(!((timer + (uint16_t) &request) % SETTINGS_GATHER_PERIOD))
    {
        put_gather_message();
    }

    _MODE_MONITOR({
        put_debug_assignment_message();
    });

    control_listen_until(&random_slot);

    bool time_left = control_speak_until(deadline);
    if(time_left)
        control_listen_until(deadline);
}

inline
void data_handle_messages(
    Time_cptr time, const uint8_t rssi,
    uint8_t *buffer_ptr, uint8_t_cptr buffer_end)
{
    DEBUG(  TIME_FMT "|R|+%ub\r\n",
            TIME_FMT_DATA(*time), buffer_end - buffer_ptr);

    uint8_t count = 0;
    while(buffer_ptr < buffer_end)
    {
        Message_cptr msg = (Message_cptr) buffer_ptr;
        switch(validate_message(msg, &buffer_ptr, buffer_end))
        {
            case KIND_DATA:
                handle_data(time, (MessageData_cptr) msg, rssi);
                ++ count;
                break;

            case KIND_SYNCHRONIZATION:
                handle_synchronization(
                    time, (MessageSynchronization_cptr) msg, rssi);

                ++ count;
                break;

            case KIND_EOF:
                ++ buffer_ptr;
                break;
        }
    }

    NOTICE(TIME_FMT "|R|+%um\r\n", TIME_FMT_DATA(*time), count);
}

void data_listen_once(void)
{
    uint8_t signal = iinic_instant_poll(IINIC_RX_COMPLETE | IINIC_SIGNAL);
    if(signal & IINIC_RX_COMPLETE)
    {
        data_handle_messages(
            (Time_cptr) &iinic_rx_timing,
            _scale_rssi(iinic_rx_rssi),
            rxbuffer, iinic_buffer_ptr);

        iinic_rx();
    }

#ifdef __USART_COMPLEX__
    if(signal & IINIC_SIGNAL)
        handle_usart();
#endif
}

void data_listen_until(Time_cptr deadline)
{
    uint8_t signal;
    while((signal = timed_poll(IINIC_RX_COMPLETE | IINIC_SIGNAL, deadline)))
    {
        if(signal & IINIC_RX_COMPLETE)
        {
            data_handle_messages(
                (Time_cptr) &iinic_rx_timing,
                _scale_rssi(iinic_rx_rssi),
                rxbuffer, iinic_buffer_ptr);

            iinic_rx();
        }

#ifdef __USART_COMPLEX__
        if(signal & IINIC_SIGNAL)
            handle_usart();
#endif
    }
}

inline
bool data_speak_until(Time_cptr deadline)
{
    if(!request.bytes_left)
        return true;

    iinic_idle();

    uint8_t size = (random() % 16) + 16;
    size = min(size, request.bytes_left / 8);
    data_txbuffer_ptr = data_txbuffer;
    put_data_message(request.destination, size);
    request.bytes_left -= size * 8;

    iinic_set_buffer(data_txbuffer, data_txbuffer_ptr - data_txbuffer);
    iinic_tx();

    bool time_left = true;
    if(!timed_poll(IINIC_TX_COMPLETE, deadline))
    {
        time_left = false;
        iinic_idle();
    }

    else
    {
        DEBUG(  TIME_FMT "|R|-%ub\r\n",
                TIME_FMT_DATA(*deadline),
                data_txbuffer_ptr - data_txbuffer);
    }

    iinic_set_buffer(rxbuffer, SETTINGS_RXBUFFER_SIZE);
    iinic_rx();
    return time_left;
}

inline
void data_step_client(Time_cptr deadline)
{
    Time deadline_part = *deadline;
    time_add_i32(&deadline_part, -SETTINGS_DATA_FRAME_TIME);

    const Assignment *assignment = request.assignment;
    if(!assignment[0].ttl)
    {
        data_listen_until(deadline);
        return;
    }

    for(uint8_t i = 0, j = 1; i < 16; i = j, ++ j)
    {
        if(assignment->slotmask & _BV(i))
        {
            time_add_i32(   &deadline_part,
                            (int32_t) SETTINGS_DATA_FRAME_TIME / 16);

            if(!data_speak_until(&deadline_part))
                continue;
        }

        else
        {
            while(j < 16 && !(assignment->slotmask & _BV(j)))
                ++ j;

            time_add_i32(   &deadline_part,
                            (int32_t) (j - i) * SETTINGS_DATA_FRAME_TIME / 16);
        }

        data_listen_until(&deadline_part);
    }
}

inline
void _gather_neighbours(uint16_t neighbour[SETTINGS_MAX_NODES])
{
    const Edge *edge = neighbours.edge;
    for(uint8_t e = 0; e < SETTINGS_MAX_EDGES; ++ e)
    {
        if(!edge[e].ttl)
            continue;

        neighbour[edge[e].source]       |= _BV(edge[e].destination);
        neighbour[edge[e].destination]  |= _BV(edge[e].source);
    }
}

inline
void _color_neighbours(uint16_t neighbour[SETTINGS_MAX_NODES])
{
    Node *node = neighbours.node;
    Request *queue = request.queue;

    shuffle((uint8_t *) queue, request.queue_size, sizeof(Request));
    data_listen_once();

    for(uint8_t r = 0; r < request.queue_size; ++ r)
        node[queue[r].source].color = 0xFF;

    for(uint8_t r = 0; r < request.queue_size; ++ r)
    {
        uint16_t color_mask = 0;
        for(uint8_t n = 0; n < SETTINGS_MAX_NODES; ++ n)
            if(neighbour[queue[r].source] & _BV(n))
            {
                if(node[n].color < 16)
                    color_mask |= _BV(node[n].color);

                for(uint8_t n2 = 0; n2 < SETTINGS_MAX_NODES; ++ n2)
                    if(neighbour[n] & _BV(n2) && node[n2].color < 16)
                        color_mask |= _BV(node[n2].color);
            }

        node[queue[r].source].color = __builtin_ctz(~color_mask);
    }

    for(uint8_t n = 0; n < SETTINGS_MAX_NODES; ++ n)
    {
        if(node[n].color < 16)
            continue;

        uint16_t color_mask = 0;
        for(uint8_t s = 0; s < SETTINGS_MAX_NODES; ++ s)
            if(neighbour[n] & _BV(s))
            {
                if(node[s].color < 16)
                    color_mask |= _BV(node[s].color);

                for(uint8_t n2 = 0; n2 < SETTINGS_MAX_NODES; ++ n2)
                    if(neighbour[s] & _BV(n2))
                        color_mask |= _BV(node[n2].color);
            }

        node[n].color = __builtin_ctz(~color_mask);
    }
}

inline
void _sort_requests_by_priority(void)
{
    Assignment *assignment = request.assignment;
    Request *queue = request.queue;
    bool done;
    do
    {
        done = true;
        for(uint8_t r = 1; r < request.queue_size; ++ r)
            if(assignment[queue[r - 1].source].priority <
                assignment[queue[r].source].priority)
            {
                done = false;
                swap(   (uint8_t *) &queue[r - 1],
                        (uint8_t *) &queue[r],
                        sizeof(Request));
            }
    }
    while(!done);
}

inline
void _redo_existing_assignments(void)
{
    Assignment *assignment = request.assignment;
    const Node *node = neighbours.node;
    Request *queue = request.queue;
    for(uint8_t r = 0; r < request.queue_size; ++ r)
    {
        uint8_t ttl = 255;
        uint16_t slotmask = 0;
        for(uint8_t n = 0 ; n < SETTINGS_MAX_NODES; ++ n)
        {
            if(assignment[n].ttl < SETTINGS_MAX_HOP)
                continue;

            if(node[n].color != node[queue[r].source].color)
                continue;

            slotmask |= assignment[n].slotmask;
            ttl = min(ttl, assignment[n].ttl);
        }

        data_listen_once();

        if(!ttl)
            continue;

        if(!slotmask)
            continue;

        uint8_t slots = __builtin_popcountll(slotmask);
        ttl = min(  max(queue[r].size / 192 / slots,
                        SETTINGS_MAX_HOP * 8),
                    ttl);

        uint32_t possible = (uint32_t) ttl * slots * 192;
        if(possible > queue[r].size)
            ttl = (slots + queue[r].size / 192) / slots;

        if(assignment[queue[r].source].priority)
            -- assignment[queue[r].source].priority;

        assignment[queue[r].source].ttl         = ttl;
        assignment[queue[r].source].slotmask    = slotmask;
        put_response_message(queue[r].source);
    }
}

inline
void _create_new_assignments(void)
{
    Assignment *assignment = request.assignment;
    Request *queue = request.queue;
    uint16_t free_slotmask = 0xFFFF;
    for(uint8_t n = 0; n < SETTINGS_MAX_NODES; ++ n)
    {
        if(!assignment[n].ttl)
            continue;

        bool valid = true;
        for(uint8_t r = 0; r < request.queue_size && valid; ++ r)
            if(!assignment[queue[r].source].ttl)
                valid = queue[r].source != n;

        if(!valid)
            continue;

        free_slotmask &= ~assignment[n].slotmask;
    }

    if(!free_slotmask)
        return;

    uint32_t size_sum = 0;
    for(uint8_t r = 0; r < request.queue_size; ++ r)
    {
        if(assignment[queue[r].source].ttl)
            continue;

        size_sum += queue[r].size;
    }

    uint8_t free_slots = __builtin_popcountll(free_slotmask);
    for(uint8_t r = 0; r < request.queue_size && free_slotmask; ++ r)
    {
        uint8_t n = queue[r].source;
        if(assignment[n].ttl)
            continue;

        uint8_t slots = max(1,
            (uint32_t) free_slots * queue[r].size / size_sum);

        uint8_t ttl = min(  max(queue[r].size / 192 / slots,
                                SETTINGS_MAX_HOP * 8),
                            255);

        uint16_t slotmask = 0;
        for(uint8_t b = 0; b < 16 && slots; ++ b)
            if(free_slotmask & _BV(b))
            {
                free_slotmask &= ~_BV(b);
                slotmask |= _BV(b);
                -- slots;
            }

        if(assignment[n].priority)
            -- assignment[n].priority;

        assignment[n].ttl       = ttl;
        assignment[n].slotmask  = slotmask;
        put_response_message(n);
    }
}

inline
void data_step_server(Time_cptr deadline)
{
    if(request.queue_size < SETTINGS_REQUEST_QUEUE_SIZE &&
        (timer + (uint16_t) request.queue) % (SETTINGS_REQUEST_QUEUE_SIZE * 4))
    {
        data_listen_until(deadline);
        return;
    }

    {
        uint16_t neighbour[SETTINGS_MAX_NODES];

        _gather_neighbours(neighbour);
        data_listen_once();

        _color_neighbours(neighbour);
        data_listen_once();
    }

    _sort_requests_by_priority();
    data_listen_once();

    _redo_existing_assignments();
    data_listen_once();

    _create_new_assignments();

    request.queue_size = 0;
    data_listen_until(deadline);
}

inline
void data_step(Time_cptr deadline)
{
#ifndef STATIC_ROOT
    if(synchronization.root.macaddr == device_macaddr)
#else
    if(STATIC_ROOT == device_macaddr)
#endif
        data_step_server(deadline);

    else
        data_step_client(deadline);

    data_listen_until(deadline);
}

#endif // __PROTOCOL_BTS_H__
