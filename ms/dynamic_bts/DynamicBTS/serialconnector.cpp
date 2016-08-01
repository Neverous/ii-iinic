#include <QByteArray>
#include <QVector>
#include <QtSerialPort/QSerialPortInfo>

#include "../protocol/messages.h"
#include "serialconnector.h"

SerialConnector::SerialConnector(QObject *parent)
    :QObject{parent}
    ,port{new QSerialPort}
    ,buffer{}
    ,debug_buffer{}
{
    port->connect(
        port,
        SIGNAL(readyRead()),
        this,
        SLOT(on_serial_port_read())
    );

    port->connect(
        port,
        SIGNAL(error(QSerialPort::SerialPortError)),
        this,
        SLOT(on_serial_port_error(QSerialPort::SerialPortError))
    );

    startTimer(15000);
}

SerialConnector::~SerialConnector()
{
    delete port;
}

QList<QSerialPortInfo> SerialConnector::available_ports()
{
    return QSerialPortInfo::availablePorts();
}

void SerialConnector::set_port_name(const QString &port_name)
{
    port->setPortName(port_name);
}

bool SerialConnector::set_baud_rate(qint32 baud_rate)
{
    return port->setBaudRate(baud_rate);
}

QString SerialConnector::get_error() const
{
    return port->errorString();
}

bool SerialConnector::connect()
{
    return port->open(QIODevice::ReadWrite);
}

void SerialConnector::disconnect()
{
    port->close();
}

bool SerialConnector::is_connected()
{
    Q_ASSERT(port);
    return port->isOpen();
}

void SerialConnector::set_control_mode(ControlMode _mode)
{
    mode = _mode;
}

void SerialConnector::set_option(Option _option)
{
    option = _option;
}

void SerialConnector::timerEvent(QTimerEvent *)
{
    write_message_ping(mode | option);
}

bool SerialConnector::write_message_ping(quint8 options)
{
    uint8_t buffer[8] = {};
    MessagePing *msg = (MessagePing *) buffer;
    msg->kind		= KIND_PING;
    msg->options	= options;

    uint8_t size = message_ping_get_size(msg);
    *(uint16_t *) (buffer + size) = crc16(buffer, size);
    return port->write((const char *) buffer, size + 2) == size + 2;
}

void SerialConnector::handle_debug(const message_debug * const debug)
{
    switch(debug->subkind)
    {
    case SUBKIND_DEBUG_ASSIGNMENT:
        handle_debug_assignment((MessageDebugAssignment_cptr) debug);
        return;

    case SUBKIND_DEBUG_ROOT_CHANGE:
        handle_debug_root_change((MessageDebugRootChange_cptr) debug);
        return;

    case SUBKIND_DEBUG_TEXT:
        handle_debug_text((MessageDebugText_cptr) debug);
        return;
    }
}

void SerialConnector::handle_debug_assignment(const message_debug_assignment * const debug_assignment)
{
    QList<std::tuple<quint16, quint8, quint8, quint16>> assignments;
    for(int n = 0; n < SETTINGS_MAX_NODES; ++ n)
        if(debug_assignment->macaddr[n])
            assignments.append({debug_assignment->macaddr[n], debug_assignment->assignment[n].priority, debug_assignment->assignment[n].ttl, debug_assignment->assignment[n].slotmask});

    emit read_assignments(assignments);
}

void SerialConnector::handle_debug_root_change(const message_debug_root_change * const debug_root_change)
{
    emit read_root_change(debug_root_change->root_macaddr);
}

void SerialConnector::handle_debug_text(const message_debug_text * const debug_text)
{
    debug_buffer.append((const char *) debug_text->text, qstrnlen((const char *) debug_text->text, 31));
    int pos = 0;

    while((pos = debug_buffer.indexOf("\r\n")) != -1)
    {
        emit read_debug_line(debug_buffer.left(pos));
        debug_buffer.remove(0, pos + 2);
    }
}

void SerialConnector::handle_gather(const message_gather * const gather)
{
    QList<std::tuple<quint16, quint16, quint16>> stats;
    for(int n = 0; n < SETTINGS_MAX_NODES; ++ n)
        if(gather->macaddr[n])
            stats.append({gather->macaddr[n], gather->in[n], gather->out[n]});

    emit read_gather(gather->macaddr[0], gather->average_latency, stats);
}

void SerialConnector::handle_neighbours(const message_neighbours * const neighbours)
{
    QList<std::tuple<quint16, quint8, quint8>> list;
    for(int n = 0; n < neighbours->count; ++ n)
    {
        auto &neighbour = neighbours->neighbour[n];
        list.append({neighbour.macaddr, neighbour.rssi, neighbour.ttl});
    }

    emit read_neighbours(neighbours->macaddr, list);
}

void SerialConnector::on_serial_port_read()
{
    buffer.append(port->readAll());
    uint8_t *buffer_ptr = (uint8_t *) buffer.data();
    uint8_t_cptr buffer_end = buffer_ptr + buffer.size();

    bool done = false;
    while(!done && buffer_ptr < buffer_end)
    {
        Message_cptr msg = (Message_cptr) buffer_ptr;
        switch(validate_message(msg, &buffer_ptr, buffer_end))
        {
        case KIND_DEBUG:
            handle_debug((MessageDebug_cptr) msg);
            break;

        case KIND_NEIGHBOURS:
            handle_neighbours((MessageNeighbours_cptr) msg);
            break;

        case KIND_GATHER:
            handle_gather((MessageGather_cptr) msg);
            break;

        case KIND_INVALID:
            break;

        case KIND_EOF:
            done = true;
            break;

        default:
            break;
        }
    }

    buffer.remove(0, buffer_ptr - (uint8_t *) buffer.data());
}

void SerialConnector::on_serial_port_error(QSerialPort::SerialPortError code)
{
    if(code != QSerialPort::SerialPortError::NoError)
        emit error(port->errorString());
}
