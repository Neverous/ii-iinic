#ifndef __PROTOCOL_RANDOM_H__
#define __PROTOCOL_RANDOM_H__

#include <avr/pgmspace.h>

#include "blinker.h"
#include "messages.h"
#include "usart.h"

void control_step(Time_cptr deadline);
void data_step(Time_cptr deadline);

inline
static
void random_loop(void)
{
    iinic_idle();
    iinic_set_buffer(rxbuffer, SETTINGS_RXBUFFER_SIZE);
    iinic_rx();

    Time now; time_get_now(&now);
    INFO("\r\n" TIME_FMT "|M|RANDOM\r\n", TIME_FMT_DATA(now));

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
void data_step(Time_cptr deadline)
{
    Time random_slot = *deadline;
    time_add_i32(&random_slot, -(random() % SETTINGS_DATA_FRAME_TIME));
    data_listen_until(&random_slot);

    bool time_left = data_speak_until(deadline);
    if(time_left)
        control_listen_until(deadline);
}

#endif // __PROTOCOL_RANDOM_H__
