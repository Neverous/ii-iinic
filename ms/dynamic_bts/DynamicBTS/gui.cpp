#include <QErrorMessage>
#include <QLabel>
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

	configure_message_neighbours_neighbours_list();

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
        {MODE_MONITOR_MASTER,	"MMA",	ui->mode_monitor_master},
		{MODE_MASTER,			"MAS",	ui->mode_master},
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

void GUI::configure_message_neighbours_neighbours_list()
{
	auto neighbours_list = new QStandardItemModel{0, 3, this};
	neighbours_list->setHeaderData(0, Qt::Horizontal, QObject::tr("MAC Address"));
	neighbours_list->setHeaderData(1, Qt::Horizontal, QObject::tr("RSSI"));
	neighbours_list->setHeaderData(2, Qt::Horizontal, QObject::tr("TTL"));

	ui->message_neighbours_neighbours_box->setModel(neighbours_list);
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

void GUI::on_mode_master_toggled(bool value)
{
	if(!value) return;
	change_control_mode(MODE_MASTER);

}

void GUI::on_mode_monitor_master_toggled(bool value)
{
	if(!value) return;
    change_control_mode(MODE_MONITOR_MASTER);
}

void GUI::on_mode_hidden_toggled(bool value)
{
	if(!value) return;
	change_control_mode(MODE_HIDDEN);

}

void GUI::on_message_data_send_clicked()
{
	quint16 mac_address = ui->message_data_macaddr_box->value();
	quint16 destination_mac_address = ui->message_data_destination_macaddr_box->value();
	const QString &data = ui->message_data_data_box->toPlainText();

	if(!connector->write_message_data(mac_address, destination_mac_address, data.toLatin1().data(), data.length()))
		emit on_error(QObject::tr("Couldn't send data message"));

	else
		emit on_info(QObject::tr("Sent data message"));
}

void GUI::on_message_debug_send_clicked()
{
	const QString &data = ui->message_debug_debug_box->text();

	if(!connector->write_message_debug(data.toLatin1().data(), data.length()))
		emit on_error(QObject::tr("Couldn't send debug message"));

	else
		emit on_info(QObject::tr("Sent debug message"));

}

void GUI::on_message_neighbourhood_send_clicked()
{
	if(!connector->write_message_neighbourhood())
		emit on_error(QObject::tr("Couldn't send neighbourhood message"));

	else
		emit on_info(QObject::tr("Sent neighbourhood message"));
}

void GUI::on_message_neighbours_send_clicked()
{
	quint16 mac_address = ui->message_neighbours_macaddr_box->value();
    QList<std::tuple<quint16, quint8, quint8>> neighbours;

	const auto &neighbours_list = ui->message_neighbours_neighbours_box->model();
	int rows = neighbours_list->rowCount();
	for(int r = 0; r < rows; ++ r)
	{
		quint16 neighbour_mac_address	= neighbours_list->index(r, 0).data().toInt();
		quint16 neighbour_rssi			= neighbours_list->index(r, 1).data().toInt();
		quint8 neighbour_ttl			= neighbours_list->index(r, 2).data().toInt();
		neighbours.append({neighbour_mac_address, neighbour_rssi, neighbour_ttl});
	}

	if(!connector->write_message_neighbours(mac_address, neighbours))
		emit on_error(QObject::tr("Couldn't send neighbours message"));

	else
		emit on_info(QObject::tr("Sent neighbours message"));
}

void GUI::on_message_pingpong_send_clicked()
{
	quint8 options = 0;
	if(ui->message_pingpong_type_box->isChecked())
        options |= 1;

	if(ui->buttonGroup->checkedButton() == ui->message_pingpong_mode_monitor)
        options |= MODE_MONITOR;

    else if(ui->buttonGroup->checkedButton() == ui->message_pingpong_mode_monitor_master)
        options |= MODE_MONITOR_MASTER;

	else if(ui->buttonGroup->checkedButton() == ui->message_pingpong_mode_master)
        options |= MODE_MASTER;

	if(!connector->write_message_pingpong(options))
		emit on_error(QObject::tr("Couldn't send pingpong message"));

	else
		emit on_info(QObject::tr("Sent pingpong message"));

}

void GUI::on_message_request_send_clicked()
{
	quint16 mac_address = ui->message_request_macaddr_box->value();
	quint8 seq_id		= ui->message_request_seq_id_box->value();
	quint16 count		= ui->message_request_count_box->value();
	quint8 ttl			= ui->message_request_ttl_box->value();

	if(!connector->write_message_request(mac_address, seq_id, count, ttl))
		emit on_error(QObject::tr("Couldn't send request message"));

	else
		emit on_info(QObject::tr("Sent request message"));

}

void GUI::on_message_response_send_clicked()
{
	quint16 mac_address		= ui->message_response_macaddr_box->value();
	quint8 assignment_ttl	= ui->message_response_assignment_ttl_box->value();
	quint8 ttl				= ui->message_response_ttl_box->value();
	quint8 len				= ui->message_response_slotmask_box->text().length();
	quint64 slotmask		= ui->message_response_slotmask_box->value();

	if(!connector->write_message_response(mac_address, assignment_ttl, len, slotmask, ttl))
		emit on_error(QObject::tr("Couldn't send response message"));

	else
		emit on_info(QObject::tr("Sent response message"));

}

void GUI::on_message_synchronization_send_clicked()
{
	quint16 mac_address			= ui->message_synchronization_macaddr_box->value();
	quint16 root_mac_address	= ui->message_synchronization_root_macaddr_box->value();
	quint16 seq_id				= ui->message_synchronization_seq_id_box->value();
	quint64 global_time			= ui->message_synchronization_global_time_box->value();

	if(!connector->write_message_synchronization(mac_address, root_mac_address, seq_id, global_time))
		emit on_error(QObject::tr("Couldn't send synchronization message"));

	else
		emit on_info(QObject::tr("Sent synchronization message"));

}

void GUI::on_message_neighbours_remove_button_clicked()
{
	const auto &neighbours_box = ui->message_neighbours_neighbours_box->model();
	const auto &neighbours_selection = ui->message_neighbours_neighbours_box->selectionModel();
	if(!neighbours_selection->hasSelection())
		return;

	int row = neighbours_selection->selectedRows().first().row();
	neighbours_box->removeRow(row);
}

void GUI::on_message_neighbours_add_button_clicked()
{
	QString neighbour_mac_address	= ui->message_neighbours_add_macaddr_box->text();
	QString neighbour_rssi			= ui->message_neighbours_add_rssi_box->text();
	QString neighbour_ttl			= ui->message_neighbours_add_ttl_box->text();

	auto neighbours_list = (QStandardItemModel *) ui->message_neighbours_neighbours_box->model();
	int row = neighbours_list->rowCount();
	neighbours_list->insertRow(row);
	neighbours_list->setItem(row, 0, new QStandardItem{neighbour_mac_address});
	neighbours_list->setItem(row, 1, new QStandardItem{neighbour_rssi});
	neighbours_list->setItem(row, 2, new QStandardItem{neighbour_ttl});
}

void GUI::on_tab_box_tabBarClicked(int index)
{
	if(	index == ui->tab_box->currentIndex() &&
			ui->tab_box->maximumHeight() != 26)
		ui->tab_box->setMaximumHeight(26);

	else
		ui->tab_box->setMaximumHeight(16777215);
}

void GUI::on_messages_box_tabBarClicked(int index)
{
	if(	index == ui->messages_box->currentIndex() &&
			ui->messages_box->maximumWidth() != 26)
		ui->messages_box->setMaximumWidth(26);

	else
		ui->messages_box->setMaximumWidth(16777215);
}

void GUI::on_tab_box_currentChanged(int)
{
	ui->tab_box->setMaximumHeight(16777215);
}

void GUI::on_messages_box_currentChanged(int)
{
	ui->messages_box->setMaximumWidth(16777215);
}

void GUI::on_message_pingpong_type_box_toggled(bool checked)
{
	if(checked)
		ui->message_pingpong_type_box->setText(QObject::tr("Pong"));

	else
		ui->message_pingpong_type_box->setText(QObject::tr("Ping"));
}
