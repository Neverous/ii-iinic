#include <QByteArray>
#include <QtSerialPort/QSerialPortInfo>

#include "serialconnector.h"
#include "../protocol/messages.h"

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

bool SerialConnector::write_message_data(quint16 mac_address, quint16 destination_mac_address, const char *data, quint16 len)
{
    Q_ASSERT(len <= 248);
    uint8_t buffer[256] = {};
    MessageData *msg = (MessageData *) buffer;
    msg->kind			= KIND_DATA;
    msg->size			= (len + 7) / 8;
    msg->macaddr		= mac_address;
    msg->dst_macaddr	= destination_mac_address;
    memcpy(msg->data, data, len);

    uint8_t size = message_data_get_size(msg);
    *(uint16_t *) (buffer + size) = crc16(buffer, size);
    return port->write((const char *) buffer, size + 2) == size + 2;
}

bool SerialConnector::write_message_debug(const char *debug, quint8 len)
{
    Q_ASSERT(len <= 31);

    uint8_t buffer[64] = {};
    MessageDebug *msg = (MessageDebug *) buffer;
    msg->kind		= KIND_DEBUG;
    msg->kind_high	= 0x4;
    memcpy(msg->text, debug, len);

    uint8_t size = message_debug_get_size(msg);
    *(uint16_t *) (buffer + size) = crc16(buffer, size);
    return port->write((const char *) buffer, size + 2) == size + 2;

}

bool SerialConnector::write_message_neighbourhood()
{
    uint8_t buffer[2] = {};
    MessageNeighbourhood *msg = (MessageNeighbourhood *) buffer;
    msg->kind = KIND_NEIGHBOURHOOD;

    uint8_t size = message_neighbourhood_get_size(msg);
    *(uint16_t *) (buffer + size) = crc16(buffer, size);
    return port->write((const char *) buffer, size + 2) == size + 2;
}

bool SerialConnector::write_message_neighbours(quint16 mac_address, const QList<std::tuple<quint16, quint8, quint8>> &neighbours)
{
    Q_ASSERT(neighbours.length() <= 31);

    uint8_t buffer[256] = {};
    MessageNeighbours *msg = (MessageNeighbours *) buffer;
    msg->kind		= KIND_NEIGHBOURS;
    msg->count		= neighbours.length();
    msg->macaddr	= mac_address;

    int n = 0;
    for(const auto &neighbour: neighbours)
    {
        quint16 neighbour_mac_address;
        quint8 neighbour_rssi;
        quint8 neighbour_ttl;

        std::tie(neighbour_mac_address, neighbour_rssi, neighbour_ttl) = neighbour;
        msg->neighbour[n] = {neighbour_mac_address, neighbour_rssi, neighbour_ttl};
        ++ n;
    }

    uint8_t size = message_neighbours_get_size(msg);
    *(uint16_t *) (buffer + size) = crc16(buffer, size);
    return port->write((const char *) buffer, size + 2) == size + 2;
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

bool SerialConnector::write_message_request(quint16 mac_address, quint16 count, quint8 ttl)
{
    Q_ASSERT(count <= 4096);

    uint8_t buffer[16] = {};
    MessageRequest *msg = (MessageRequest *) buffer;
    msg->kind		= KIND_REQUEST;
    msg->count_high = count >> 8;
    msg->count_low	= count;
    msg->macaddr	= mac_address;
    msg->ttl		= ttl;

    uint8_t size = message_request_get_size(msg);
    *(uint16_t *) (buffer + size) = crc16(buffer, size);
    return port->write((const char *) buffer, size + 2) == size + 2;
}

bool SerialConnector::write_message_response(quint16 mac_address, quint8 assignment_ttl, quint8 len, quint32 slotmask, quint8 ttl)
{
    Q_ASSERT(len <= 64);

    uint8_t buffer[32] = {};
    MessageResponse *msg = (MessageResponse *) buffer;
    msg->kind			= KIND_RESPONSE;
    msg->macaddr		= mac_address;
    msg->ttl			= ttl;
    msg->assignment_ttl	= assignment_ttl;
    msg->slotmask       = slotmask;

    uint8_t size = message_response_get_size(msg);
    *(uint16_t *) (buffer + size) = crc16(buffer, size);
    return port->write((const char *) buffer, size + 2) == size + 2;
}

bool SerialConnector::write_message_synchronization(quint16 mac_address, quint16 root_mac_address, quint16 seq_id, quint64 global_time)
{
    uint8_t buffer[32] = {};
    MessageSynchronization *msg = (MessageSynchronization *) buffer;
    msg->kind			= KIND_SYNCHRONIZATION;
    msg->macaddr		= mac_address;
    msg->root_macaddr	= root_mac_address;
    msg->seq_id			= seq_id;
    (*(quint64 *) &msg->global_time) = global_time;

    uint8_t size = message_synchronization_get_size(msg);
    *(uint16_t *) (buffer + size) = crc16(buffer, size);
    return port->write((const char *) buffer, size + 2) == size + 2;
}

void SerialConnector::timerEvent(QTimerEvent *)
{
    write_message_ping(mode | option);
}

void SerialConnector::handle_debug(const message_debug * const debug)
{
    debug_buffer.append((const char *) debug->text, qstrnlen((const char *) debug->text, 31));
    int pos = 0;

    while((pos = debug_buffer.indexOf("\r\n")) != -1)
    {
        emit read_debug_line(debug_buffer.left(pos));
        debug_buffer.remove(0, pos + 2);
    }
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
