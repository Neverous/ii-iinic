#ifndef __PROTOCOL_BTS_H__
#define __PROTOCOL_BTS_H__

#include "blinker.h"
#include "messages.h"
#include "usart.h"

void control_step(Time_cptr deadline);
void data_step(Time_cptr deadline);

static
void bts_loop(void)
{
    iinic_idle();
    iinic_set_buffer(rxbuffer, SETTINGS_RXBUFFER_SIZE);
    iinic_rx();

    Time now; time_get_now(&now);
    NOTICE("\r\n" TIME_FMT "|M|BTS\r\n", TIME_FMT_DATA(now));

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
        validate_pingpong();
        validate_request();
        validate_synchronization();
    }
}

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
            case KIND_SYNCHRONIZATION:
                handle_synchronization(
                    time, (MessageSynchronization_cptr) msg, rssi);

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

            case KIND_EOF:
                ++ buffer_ptr;
                break;
        }
    }

    DEBUG(TIME_FMT "|R|+%um\r\n", TIME_FMT_DATA(*time), count);
}

void control_listen_until(Time_cptr deadline)
{
    uint8_t signal;
    while((signal = timed_poll( IINIC_RX_COMPLETE | IINIC_SIGNAL,
                                deadline)))
    {
        if(signal & IINIC_RX_COMPLETE)
        {
            control_handle_messages(
                (Time_cptr) &iinic_rx_timing,
                _scale_rssi(iinic_rx_rssi),
                rxbuffer, iinic_buffer_ptr);

            iinic_rx();
        }

        if(signal & IINIC_SIGNAL)
            handle_usart();
    }
}

bool control_speak_until(Time_cptr deadline)
{
    if( (   synchronization.timer.valid ||
#ifndef STATIC_ROOT
            synchronization.root.macaddr == device_macaddr) &&
#else
            STATIC_ROOT == device_macaddr) &&
#endif
        (   synchronization.timer.trigger ||
            !(synchronization.timer.counter % SETTINGS_SYNCHRONIZATION_PERIOD)))
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

    iinic_set_buffer(rxbuffer, SETTINGS_RXBUFFER_SIZE);
    iinic_rx();
    control_txbuffer_ptr = control_txbuffer;
    return time_left;
}

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

    if(!(neighbours.timer % SETTINGS_NEIGHBOURS_PERIOD))
    {
        put_neighbours_message();
    }

    control_listen_until(&random_slot);

    bool time_left = control_speak_until(deadline);
    if(time_left)
        control_listen_until(deadline);
}

void data_handle_messages(
    Time_cptr time, __unused__ const uint8_t rssi,
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
                ++ count;
                break;

            case KIND_EOF:
                ++ buffer_ptr;
                break;
        }
    }

    DEBUG(TIME_FMT "|R|+%um\r\n", TIME_FMT_DATA(*time), count);
}

void data_listen_until(Time_cptr deadline)
{
    uint8_t signal;
    while((signal = timed_poll( IINIC_RX_COMPLETE | IINIC_SIGNAL,
                                deadline)))
    {
        if(signal & IINIC_RX_COMPLETE)
        {
            data_handle_messages(
                (Time_cptr) &iinic_rx_timing,
                _scale_rssi(iinic_rx_rssi),
                rxbuffer, iinic_buffer_ptr);

            iinic_rx();
        }

        if(signal & IINIC_SIGNAL)
            handle_usart();
    }
}

bool data_speak_until(Time_cptr deadline)
{
    if(data_txbuffer_ptr == data_txbuffer)
        return true;

    iinic_idle();
    iinic_set_buffer(data_txbuffer, data_txbuffer_ptr - data_txbuffer);
    iinic_tx();

    bool time_left = true;
    if(!timed_poll(IINIC_TX_COMPLETE, deadline))
    {
        time_left = false;
        iinic_idle();
    }

    iinic_set_buffer(rxbuffer, SETTINGS_RXBUFFER_SIZE);
    iinic_rx();
    data_txbuffer_ptr = data_txbuffer;
    return time_left;
}

void data_step(Time_cptr deadline)
{
    Time deadline_part = *deadline;
    time_add_i32(   &deadline_part,
                    -SETTINGS_DATA_FRAME_TIME);

    if(!request.assignment[0].ttl)
    {
        data_listen_until(deadline);
        return;
    }

    Assignment *assignment = request.assignment;
    uint8_t step = 8 / assignment[0].size;
    for(uint8_t i = 0, j = 1;
        i * step < SETTINGS_DATA_SLOTS &&
        time_cmp_now(deadline) < 0;
        i = j, ++ j)
    {
        bool speak = assignment->slotmask[i / 8] & (i << (i % 8));
        while(j * step < SETTINGS_DATA_SLOTS &&
            !speak == !(assignment->slotmask[j / 8] & (1 << (j % 8))))
            ++ j;

        time_add_i32(&deadline_part,
            (int32_t) (j - i) * step * SETTINGS_DATA_SLOT_TIME);

        if(speak && !data_speak_until(&deadline_part))
            continue;

        data_listen_until(&deadline_part);
    }

    while(time_cmp_now(deadline) < 0);
}

#endif // __PROTOCOL_BTS_H__
