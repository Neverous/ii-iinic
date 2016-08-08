#ifndef SERIALPORTSELECTOR_H
#define SERIALPORTSELECTOR_H

#include <QDialog>

class QErrorMessage;
class SerialConnector;

namespace Ui
{
class SerialPortSelector;
}

class SerialPortSelector: public QDialog
{
    Q_OBJECT

private:
    Ui::SerialPortSelector  *ui;

public:
    explicit SerialPortSelector(QWidget *parent=nullptr);
    ~SerialPortSelector();

protected:
    void showEvent(QShowEvent *);

private:
    void configure_serial_ports_list();
    void populate_serial_ports_list();

private slots:
    void _on_button_box_refresh();
    void on_buttons_accepted();

signals:
    void serial_port_selected(const QString &port_name, qint32 baud_rate);
};

#endif // SERIALPORTSELECTOR_H
