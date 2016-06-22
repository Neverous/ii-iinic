#include <QErrorMessage>
#include <QPushButton>
#include <QStandardItemModel>
#include <QtSerialPort/QSerialPortInfo>

#include "serialportselector.h"
#include "ui_serialportselector.h"
#include "serialconnector.h"

SerialPortSelector::SerialPortSelector(QWidget *parent)
    :QDialog{parent}
    ,ui{new Ui::SerialPortSelector}
    ,error{new QErrorMessage{this}}
{
    ui->setupUi(this);

    auto refresh = ui->buttons->button(QDialogButtonBox::Reset);
    refresh->setText(QObject::tr("Refresh"));
    connect(
        refresh,
        SIGNAL(clicked()),
        this,
        SLOT(on_button_box_refresh())
    );

    configure_serial_ports_list();
}

SerialPortSelector::~SerialPortSelector()
{
    delete error;
    delete ui;
}

void SerialPortSelector::showEvent(QShowEvent *)
{
    populate_serial_ports_list();
}

void SerialPortSelector::configure_serial_ports_list()
{
    auto serial_ports_list = new QStandardItemModel{0, 5, this};
    serial_ports_list->setHeaderData(0, Qt::Horizontal, QObject::tr("Idle"));
    serial_ports_list->setHeaderData(1, Qt::Horizontal, QObject::tr("Identifier"));
    serial_ports_list->setHeaderData(2, Qt::Horizontal, QObject::tr("Serial number"));
    serial_ports_list->setHeaderData(3, Qt::Horizontal, QObject::tr("Description"));
    serial_ports_list->setHeaderData(4, Qt::Horizontal, QObject::tr("Port"));

    ui->serial_ports_list_box->setModel(serial_ports_list);
}

void SerialPortSelector::populate_serial_ports_list()
{
    auto serial_ports_list = (QStandardItemModel *) ui->serial_ports_list_box->model();
    serial_ports_list->removeRows(0, serial_ports_list->rowCount());
    int row = 0;
    for(const auto &serial_port: SerialConnector::available_ports())
    {
        serial_ports_list->insertRow(row);
        serial_ports_list->setItem(row, 0, new QStandardItem{serial_port.isBusy() ? QObject::tr("No") : QObject::tr("Yes")});
        serial_ports_list->setItem(row, 1, new QStandardItem{serial_port.manufacturer() + QString::asprintf(" (%04x:%04x)", serial_port.vendorIdentifier(), serial_port.productIdentifier())});
        serial_ports_list->setItem(row, 2, new QStandardItem{serial_port.serialNumber()});
        serial_ports_list->setItem(row, 3, new QStandardItem{serial_port.description()});
        serial_ports_list->setItem(row, 4, new QStandardItem{serial_port.portName() + " (" + serial_port.systemLocation() + ")"});
        ++ row;
    }

    ui->serial_ports_list_box->resizeColumnsToContents();
}

void SerialPortSelector::on_button_box_refresh()
{
    populate_serial_ports_list();
    ui->serial_ports_list_box->setFocus();
}

void SerialPortSelector::on_buttons_accepted()
{
    auto selection = ui->serial_ports_list_box->selectionModel();
    if(!selection->hasSelection())
    {
        error->showMessage(QObject::tr("Please choose port"));
        return;
    }

    auto data = selection->model();
    int row = selection->selectedRows().first().row();

    QString port_name = data->index(row, 1).data().toString();
    qint32 baud_rate = ui->baud_rate_box->value();

    emit serial_port_selected(port_name, baud_rate);
    accept();
}
