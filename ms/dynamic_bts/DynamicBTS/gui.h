#ifndef GUI_H
#define GUI_H

#include <QMainWindow>

class NetworkVisualization;
class QErrorMessage;
class QLabel;
class SerialConnector;
class SerialPortSelector;

namespace Ui
{
class GUI;
}

class GUI: public QMainWindow
{
    Q_OBJECT

private:
	Ui::GUI					*ui;
	NetworkVisualization	*visualization;
	SerialConnector			*connector;
	SerialPortSelector		*selector;
	QErrorMessage			*error;
	QLabel					*mode_box;
	QWidget					*status_box;

public:
	explicit GUI(SerialConnector *_connector, QWidget *parent=nullptr);
    ~GUI();

protected:
	void showEvent(QShowEvent *event);

private:
	void change_control_mode(qint32 mode);
	void configure_message_neighbours_neighbours_list();

private slots:
	void on_serial_port_selected(const QString &port_name, qint32 baud_rate);
	void on_action_open_triggered();
	void on_action_close_triggered();
	void on_debug_line_read(const QString &debug_line);
    void on_neighbours_read(quint16 mac_address, const QList<std::tuple<quint16, quint8, quint8>> &neighbours);
	void on_error(const QString &error);
	void on_info(const QString &info);
	void on_success(const QString &success);
	void on_tab_box_tabBarClicked(int index);
	void on_mode_monitor_toggled(bool value);
	void on_mode_master_toggled(bool value);
	void on_mode_monitor_master_toggled(bool value);
	void on_mode_hidden_toggled(bool value);
	void on_message_data_send_clicked();
	void on_message_debug_send_clicked();
	void on_message_neighbourhood_send_clicked();
	void on_message_neighbours_send_clicked();
	void on_message_pingpong_send_clicked();
	void on_message_request_send_clicked();
	void on_message_response_send_clicked();
	void on_message_synchronization_send_clicked();
	void on_message_neighbours_remove_button_clicked();
	void on_message_neighbours_add_button_clicked();
	void on_messages_box_tabBarClicked(int index);
	void on_tab_box_currentChanged(int);
	void on_messages_box_currentChanged(int);
	void on_message_pingpong_type_box_toggled(bool checked);
};

#endif // GUI_H
