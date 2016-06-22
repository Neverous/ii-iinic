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
    void on_mode_hidden_toggled(bool value);
    void on_tab_box_currentChanged(int);
    void on_action_about_triggered();
};

#endif // GUI_H
