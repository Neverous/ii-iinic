#include "latencylogwidget.h"

#include <QDateTime>

LatencyLogWidget::LatencyLogWidget(QWidget *parent)
    :QTableWidget{parent}
    ,node_pos{}
{
}

void LatencyLogWidget::clear()
{
    node_pos.clear();
    QTableWidget::clear();
    setRowCount(0);
    setColumnCount(0);
}

void LatencyLogWidget::update_node(quint16 mac_address, quint16 latency)
{
    if(!latency)
        return;

    int column;
    if(!node_pos.count(mac_address))
    {
        column = node_pos[mac_address] = columnCount();
        insertColumn(column);
        setHorizontalHeaderItem(column, new QTableWidgetItem{QString::number(mac_address, 16)});
    }

    else
        column = node_pos[mac_address];

    if(!rowCount())
        insertRow(0);

    auto cell = item(0, column);
    if(!cell)
        setItem(0, column, cell = new QTableWidgetItem{});

    cell->setText(QString::number(latency));

    check_row();
}

void LatencyLogWidget::check_row()
{
    int columns = columnCount();
    bool valid = true;
    for(int c = 0; c < columns && valid; ++ c)
        valid = item(0, c);

    if(valid)
    {
        setVerticalHeaderItem(0, new QTableWidgetItem{QDateTime::currentDateTime().toString()});
        insertRow(0);
    }
}
