#ifndef SERIALCONNECTOR_H
#define SERIALCONNECTOR_H

#include <QObject>
#include <QSerialPort>

struct message_debug;
struct message_neighbours;

enum ControlMode
{
	MODE_HIDDEN			= 0,
    MODE_MONITOR		= 2,
    MODE_MONITOR_MASTER	= 4,
    MODE_MASTER			= 6,
};

class SerialConnector: public QObject
{
    Q_OBJECT

private:
	QSerialPort	*port;
	QByteArray	buffer;
	QByteArray	debug_buffer;
    ControlMode mode;

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

	bool write_message_data(quint16 mac_address, quint16 destination_mac_address, const char* data, quint16 len);
	bool write_message_debug(const char* debug, quint8 len);
	bool write_message_neighbourhood();
    bool write_message_neighbours(quint16 mac_address, const QList<std::tuple<quint16, quint8, quint8> > &neighbours);
	bool write_message_pingpong(quint8 options);
	bool write_message_request(quint16 mac_address, quint8 seq_id, quint16 count, quint8 ttl);
	bool write_message_response(quint16 mac_address, quint8 assignment_ttl, quint8 len, quint64 slotmask, quint8 ttl);
	bool write_message_synchronization(quint16 mac_address, quint16 root_mac_address, quint16 seq_id, quint64 global_time);

protected:
    void timerEvent(QTimerEvent *) Q_DECL_OVERRIDE;

private:
	void handle_debug(const message_debug * const debug);
	void handle_neighbours(const message_neighbours * const neighbours);

private slots:
	void on_serial_port_read();
	void on_serial_port_error(QSerialPort::SerialPortError code);

signals:
	void connected();
	void disconnected();
	void error(const QString &error);

	void read_debug_line(const QString &debug_line);
    void read_neighbours(quint16 mac_address, const QList<std::tuple<quint16, quint8, quint8>> &neighbours);
};

#endif // SERIALCONNECTOR_H
