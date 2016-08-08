#ifndef GUI_H
#define GUI_H

#include <QMainWindow>
#include <QMap>

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
    Ui::GUI                 *ui;
    NetworkVisualization    *visualization;
    SerialConnector         *connector;
    SerialPortSelector      *selector;
    QLabel                  *mode_box;
    QWidget                 *status_box;

public:
    explicit GUI(SerialConnector *_connector, QWidget *parent=nullptr);
    ~GUI();

protected:
    void showEvent(QShowEvent *event);

private:
    void change_control_mode(qint32 mode);
    void configure_message_neighbours_neighbours_list();

private slots:
    void _on_serial_port_selected(const QString &port_name, qint32 baud_rate);
    void on_action_open_triggered();
    void on_action_close_triggered();
    void _on_debug_line_read(const QString &debug_line);
    void on_assignments(const QList<std::tuple<quint16, quint8, quint8, quint16>> &_assignments);
    void on_gather(quint16 source_mac_address, quint16 latency, const QList<std::tuple<quint16, quint16, quint16> > &stats);
    void _on_root_change(quint16 root_mac_address);
    void _on_neighbours_read(quint16 mac_address, const QList<std::tuple<quint16, quint8, quint8>> &neighbours);
    void on_error(const QString &error);
    void on_info(const QString &info);
    void on_success(const QString &success);
    void on_tab_box_tabBarClicked(int index);
    void on_mode_monitor_toggled(bool value);
    void on_mode_hidden_toggled(bool value);
    void on_tab_box_currentChanged(int);
    void on_action_about_triggered();
    void on_action_export_log_triggered();
    void on_action_export_data_stats_triggered();
    void on_action_export_latency_stats_triggered();
    void on_action_export_graph_triggered();
    void on_action_about_qt_triggered();
};

#endif // GUI_H
