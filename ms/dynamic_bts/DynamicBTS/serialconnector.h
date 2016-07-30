#ifndef SERIALCONNECTOR_H
#define SERIALCONNECTOR_H

#include <QObject>
#include <QSerialPort>

struct message_debug;
struct message_debug_assignment;
struct message_debug_node_speak;
struct message_debug_root_change;
struct message_debug_text;
struct message_gather;
struct message_neighbours;

enum ControlMode
{
    MODE_HIDDEN			= 0,
    MODE_MONITOR		= 1,
};

enum Option
{
    OPTION_MASTER       = 2,
};

class SerialConnector: public QObject
{
    Q_OBJECT

private:
    QSerialPort	*port;
    QByteArray	buffer;
    QByteArray	debug_buffer;
    ControlMode mode;
    Option      option;

public:
    explicit SerialConnector(QObject *parent=nullptr);
    ~SerialConnector();

    static QList<QSerialPortInfo> available_ports();

    void set_port_name(const QString &port_name);
    bool set_baud_rate(qint32 baud_rate);
    QString get_error() const;

    bool connect();
    void disconnect();

    bool is_connected();

    void set_control_mode(ControlMode _mode);
    void set_option(Option _option);

    bool write_message_data(quint16 mac_address, quint16 destination_mac_address, const char* data, quint16 len);
    bool write_message_debug_text(const char* debug, quint8 len);
    bool write_message_neighbourhood();
    bool write_message_neighbours(quint16 mac_address, const QList<std::tuple<quint16, quint8, quint8> > &neighbours);
    bool write_message_ping(quint8 options);
    bool write_message_request(quint16 mac_address, quint16 count, quint8 ttl);
    bool write_message_response(quint16 mac_address, quint8 assignment_ttl, quint8 len, quint32 slotmask, quint8 ttl);
    bool write_message_synchronization(quint16 mac_address, quint16 root_mac_address, quint16 seq_id, quint64 global_time);

protected:
    void timerEvent(QTimerEvent *) Q_DECL_OVERRIDE;

private:
    void handle_debug(const message_debug * const debug);
    void handle_debug_assignment(const message_debug_assignment * const debug_assignment);
    void handle_debug_node_speak(const message_debug_node_speak * const debug_node_speak);
    void handle_debug_root_change(const message_debug_root_change * const debug_root_change);
    void handle_debug_text(const message_debug_text * const debug_text);
    void handle_gather(const message_gather * const gather);
    void handle_neighbours(const message_neighbours * const neighbours);

private slots:
    void on_serial_port_read();
    void on_serial_port_error(QSerialPort::SerialPortError code);

signals:
    void connected();
    void disconnected();
    void error(const QString &error);

    void read_assignments(const QList<std::tuple<quint16, quint8, quint8, quint16>> &assignments);
    void read_node_speak(quint16 mac_address, quint8 bytes);
    void read_root_change(quint16 root_mac_address);
    void read_debug_line(const QString &debug_line);
    void read_gather(const QList<std::tuple<quint16, quint16, quint16>> &stats);
    void read_neighbours(quint16 mac_address, const QList<std::tuple<quint16, quint8, quint8>> &neighbours);
};

#endif // SERIALCONNECTOR_H
