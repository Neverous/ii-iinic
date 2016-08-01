#include <QErrorMessage>
#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QTextStream>

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
    ui->graph_box->setScene(visualization);

    ui->status_bar->addPermanentWidget(mode_box);
    change_control_mode(MODE_HIDDEN);

    connect(
        selector,
        SIGNAL(serial_port_selected(QString, qint32)),
        this,
        SLOT(on_serial_port_selected(QString, qint32))
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
        SIGNAL(read_neighbours(quint16, QList<std::tuple<quint16, quint8, quint8>>)),
        this,
        SLOT(on_neighbours_read(quint16, QList<std::tuple<quint16, quint8, quint8>>))
    );

    connect(
        connector,
        SIGNAL(read_root_change(quint16)),
        this,
        SLOT(on_root_change(quint16))
    );

    connect(
        connector,
        SIGNAL(read_assignments(QList<std::tuple<quint16, quint8, quint8, quint16>>)),
        this,
        SLOT(on_assignments(QList<std::tuple<quint16, quint8, quint8, quint16>>))
    );

    connect(
        connector,
        SIGNAL(read_gather(quint16, quint16, QList<std::tuple<quint16, quint16, quint16>>)),
        this,
        SLOT(on_gather(quint16, quint16, QList<std::tuple<quint16, quint16, quint16>>))
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
    for(const auto &element: QList<std::tuple<ControlMode, QString, QAction *>>{
        {MODE_HIDDEN,   "HID",  ui->mode_hidden},
        {MODE_MONITOR,  "MON",  ui->mode_monitor},
        })
    {
        ControlMode mode_id;
        QString     mode_str;
        QAction     *mode_ptr;
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
    ui->data_stats_box->clear();
    ui->assignments_box->clear();
    ui->latency_box->clear();

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

void GUI::on_assignments(const QList<std::tuple<quint16, quint8, quint8, quint16>> &_assignments)
{
    ui->assignments_box->update_assignments(_assignments);
}

void GUI::on_gather(quint16 source_mac_address, quint16 latency, const QList<std::tuple<quint16, quint16, quint16> > &stats)
{
    ui->data_stats_box->update_node(source_mac_address, stats);
    ui->latency_box->update_node(source_mac_address, latency);
}

void GUI::on_root_change(quint16 root_mac_address)
{
    visualization->update_root(root_mac_address);
}

void GUI::on_neighbours_read(quint16 mac_address, const QList<std::tuple<quint16, quint8, quint8>> &neighbours)
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
    if(!value)
        return;

    change_control_mode(MODE_MONITOR);
}

void GUI::on_mode_hidden_toggled(bool value)
{
    if(!value)
        return;

    change_control_mode(MODE_HIDDEN);
}

void GUI::on_tab_box_tabBarClicked(int index)
{
    if(index == ui->tab_box->currentIndex() &&
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
    QMessageBox::about(this, QObject::tr("About DynamicBTS"),
        QObject::tr("This is simple client for debugging simple_bts devices."));
}

void GUI::on_action_export_log_triggered()
{
    QString file_name = QFileDialog::getSaveFileName(this, QObject::tr("Export log"), "debug.log", QObject::tr("Log files (%1)").arg("*.log"));
    if(file_name.isEmpty())
        return;

    QFile file{file_name};
    if(file.open(QFile::WriteOnly | QFile::Truncate))
    {
        QTextStream output{&file};
        output << ui->debug_box->document()->toPlainText();
        file.close();
    }

    else
        error->showMessage(QObject::tr("Failed to save file %1, error: %2")
                                    .arg(file_name)
                                    .arg(file.errorString()));
}

void GUI::on_action_export_data_stats_triggered()
{
    QString file_name = QFileDialog::getSaveFileName(this, QObject::tr("Export data stats"), "data.csv", QObject::tr("CSV files (%1)").arg("*.csv"));
    if(file_name.isEmpty())
        return;

    QFile file{file_name};

    if(file.open(QFile::WriteOnly | QFile::Truncate))
    {
        QTextStream output{&file};
        int columns = ui->data_stats_box->columnCount();
        for(int c = 0; c < columns; ++ c)
            output << ";\"" << ui->data_stats_box->horizontalHeaderItem(c)->text() << "\"";

        output << "\n";

        int rows = ui->data_stats_box->rowCount();
        for(int r = 0; r < rows; ++ r)
        {
            output << "\"" << ui->data_stats_box->verticalHeaderItem(r)->text() << "\"";
            for(int c = 0; c < columns; ++ c)
            {
                auto item = ui->data_stats_box->item(r, c);
                output << ";\"" << (item ? item->text() : "") << "\"";
            }

            output << "\n";
        }

        file.close();
    }

    else
        error->showMessage(QObject::tr("Failed to save file %1, error: %2")
                                    .arg(file_name)
                                    .arg(file.errorString()));
}

void GUI::on_action_export_latency_stats_triggered()
{
    QString file_name = QFileDialog::getSaveFileName(this, QObject::tr("Export latency stats"), "latency.csv", QObject::tr("CSV files (%1)").arg("*.csv"));
    if(file_name.isEmpty())
        return;

    QFile file{file_name};

    if(file.open(QFile::WriteOnly | QFile::Truncate))
    {
        QTextStream output{&file};
        int columns = ui->latency_box->columnCount();
        for(int c = 0; c < columns; ++ c)
            output << ";\"" << ui->latency_box->horizontalHeaderItem(c)->text() << "\"";

        output << "\n";

        int rows = ui->latency_box->rowCount();
        for(int r = 0; r < rows; ++ r)
        {
            auto head = ui->latency_box->verticalHeaderItem(r);
            output << "\"" << (head ? head->text() : "") << "\"";
            for(int c = 0; c < columns; ++ c)
            {
                auto item = ui->latency_box->item(r, c);
                output << ";\"" << (item ? item->text() : "") << "\"";
            }
            output << "\n";
        }

        file.close();
    }

    else
        error->showMessage(QObject::tr("Failed to save file %1, error: %2")
                                    .arg(file_name)
                                    .arg(file.errorString()));
}

void GUI::on_action_export_graph_triggered()
{
    QString file_name = QFileDialog::getSaveFileName(this, QObject::tr("Export network graph"), "network.dot", QObject::tr("GraphViz files (%1)").arg("*.dot"));
    if(file_name.isEmpty())
        return;

    QFile file{file_name};

    if(file.open(QFile::WriteOnly | QFile::Truncate))
    {
        QTextStream output{&file};
        output << "graph Network {\n";
        QSet<NetworkEdge *> edges;
        for(auto &node: visualization->get_nodes())
        {
            output << "    " << QString::number(node->get_mac_address(), 16) << ";\n";
            for(auto &edge: node->get_edges())
                edges << edge;
        }

        for(auto &edge: edges)
            output << "    " << QString::number(edge->get_source()->get_mac_address(), 16)
                   << " -- " << QString::number(edge->get_destination()->get_mac_address(), 16)
                   << " [label=\"" << QString::number(edge->get_rssi()) << "\"];\n";

        output << "}\n";
        file.close();
    }

    else
        error->showMessage(QObject::tr("Failed to save file %1, error: %2")
                                    .arg(file_name)
                                    .arg(file.errorString()));
}
