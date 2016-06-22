#include <QErrorMessage>
#include <QLabel>
#include <QMessageBox>
#include <QStandardItemModel>

#include "gui.h"
#include "networkvisualization.h"
#include "serialconnector.h"
#include "serialportselector.h"
#include "ui_gui.h"

GUI::GUI(SerialConnector *_connector, QWidget *parent)
    :QMainWindow{parent}
    ,ui{new Ui::GUI}
    ,visualization{new NetworkVisualization}
    ,connector{_connector}
    ,selector{new SerialPortSelector{this}}
    ,error{new QErrorMessage{this}}
    ,mode_box{new QLabel{this}}
    ,status_box{new QWidget{this}}
{
    ui->setupUi(this);
    ui->visualization_box->setScene(visualization);

    ui->status_bar->addPermanentWidget(mode_box);
    change_control_mode(MODE_HIDDEN);

    //status_box->setStyleSheet("background: rgb(255, 0, 0);");
    //status_box->setMinimumSize(22, 22);
    //status_box->setMaximumSize(22, 22);
    //ui->status_bar->addPermanentWidget(status_box);

    //auto node1 = new NetworkNode{0xAAAA};
    //auto node2 = new NetworkNode{0xBBBB};
    //visualization->addItem(node1);
    //visualization->addItem(node2);
    //visualization->addItem(new NetworkEdge{node1, node2, 400, 255});

    connect(
        selector,
        SIGNAL(serial_port_selected(QString,qint32)),
        this,
        SLOT(on_serial_port_selected(QString,qint32))
    );

    connect(
        connector,
        SIGNAL(read_debug_line(QString)),
        this,
        SLOT(on_debug_line_read(QString))
    );

    connect(
        connector,
        SIGNAL(error(QString)),
        this,
        SLOT(on_error(QString))
    );

    connect(
        connector,
        SIGNAL(read_neighbours(quint16,QList<std::tuple<quint16,quint8,quint8> >)),
        this,
        SLOT(on_neighbours_read(quint16,QList<std::tuple<quint16,quint8,quint8> >))
    );
}

GUI::~GUI()
{
    delete mode_box;
    delete status_box;
    delete selector;
    delete visualization;
    delete ui;

}

void GUI::showEvent(QShowEvent *event)
{
    Q_ASSERT(connector);
    if(!connector->is_connected())
    {
        selector->show();
        selector->raise();
        selector->activateWindow();
    }

    QMainWindow::showEvent(event);
}

void GUI::change_control_mode(qint32 mode)
{
    for(const auto &element: QList<std::tuple<ControlMode, QString, QAction*>>{
        {MODE_HIDDEN,			"HID",	ui->mode_hidden},
        {MODE_MONITOR,			"MON",	ui->mode_monitor},
        })
    {
        ControlMode mode_id;
        QString		mode_str;
        QAction*	mode_ptr;
        std::tie(mode_id, mode_str, mode_ptr) = element;

        if(mode != mode_id)
        {
            if(mode_ptr->isChecked())
                mode_ptr->setChecked(false);

            if(!mode_ptr->isEnabled())
                mode_ptr->setEnabled(true);
        }

        else
        {
            mode_ptr->setEnabled(false);
            mode_box->setText(mode_str);
        }
    }

    ui->visualization_box->setHidden(mode == MODE_HIDDEN);
    connector->set_control_mode((ControlMode) mode);
}

void GUI::on_serial_port_selected(const QString &port_name, qint32 baud_rate)
{
    if(connector->is_connected())
        connector->disconnect();

    visualization->clear();
    ui->debug_box->clear();

    connector->set_port_name(port_name);
    if(!connector->set_baud_rate(baud_rate))
    {
        selector->show();
        error->showMessage(
            QObject::tr("Failed to set baud rate %1, error: %2")
                    .arg(baud_rate)
                    .arg(connector->get_error()));
        return;
    }

    if(!connector->connect())
    {
        selector->show();
        error->showMessage(
            QObject::tr("Failed to open port %1, error: %2")
                    .arg(port_name)
                    .arg(connector->get_error()));
        return;
    }

    show();
    raise();
    activateWindow();
}

void GUI::on_action_open_triggered()
{
    selector->show();
}

void GUI::on_action_close_triggered()
{
    qApp->quit();
}

void GUI::on_debug_line_read(const QString &debug_line)
{
    ui->debug_box->append(debug_line);
}

void GUI::on_neighbours_read(quint16 mac_address, const QList<std::tuple<quint16, quint8, quint8> > &neighbours)
{
    visualization->update_node(mac_address, neighbours);
}

void GUI::on_error(const QString &error)
{
    ui->debug_box->append("<strong style='color: rgb(255, 0, 0);'>" + error + "</strong>");
}

void GUI::on_info(const QString &info)
{
    ui->debug_box->append("<strong style='color: rgb(255, 255, 255);'>" + info + "</strong>");
}

void GUI::on_success(const QString &success)
{
    ui->debug_box->append("<strong style='color: rgb(0, 255, 0);'>" + success + "</strong>");
}

void GUI::on_mode_monitor_toggled(bool value)
{
    if(!value) return;
    change_control_mode(MODE_MONITOR);
}

void GUI::on_mode_hidden_toggled(bool value)
{
    if(!value) return;
    change_control_mode(MODE_HIDDEN);

}

void GUI::on_tab_box_tabBarClicked(int index)
{
    if(	index == ui->tab_box->currentIndex() &&
            ui->tab_box->maximumHeight() != 26)
        ui->tab_box->setMaximumHeight(26);

    else
        ui->tab_box->setMaximumHeight(16777215);
}

void GUI::on_tab_box_currentChanged(int)
{
    ui->tab_box->setMaximumHeight(16777215);
}

void GUI::on_action_about_triggered()
{
    QMessageBox::about(this, "About DynamicBTS", "This is simple client for debugging simple_bts devices.");
}
