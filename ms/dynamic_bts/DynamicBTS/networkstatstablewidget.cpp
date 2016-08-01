#include "networkstatstablewidget.h"

NetworkStatsTableWidget::NetworkStatsTableWidget(QWidget *parent)
    :QTableWidget{parent}
    ,node_pos{}
{

}

void NetworkStatsTableWidget::clear()
{
    node_pos.clear();
    QTableWidget::clear();
    setRowCount(0);
    setColumnCount(0);
}

void NetworkStatsTableWidget::update_node(quint16 source_mac_address, const QList<std::tuple<quint16, quint16, quint16> > &stats)
{
    int source_pos;
    if(!node_pos.count(source_mac_address))
    {
        source_pos = node_pos[source_mac_address] = columnCount();
        insertColumn(source_pos);
        insertRow(source_pos);
        setHorizontalHeaderItem(source_pos, new QTableWidgetItem{QString::number(source_mac_address, 16)});
        setVerticalHeaderItem(source_pos, new QTableWidgetItem{QString::number(source_mac_address, 16)});
    }

    else
        source_pos = node_pos[source_mac_address];

    for(auto &stat: stats)
    {
        quint16 destination_mac_address;
        quint16 in;
        quint16 out;

        std::tie(destination_mac_address, in, out) = stat;

        int destination_pos;
        if(!node_pos.count(destination_mac_address))
        {
            destination_pos = node_pos[destination_mac_address] = columnCount();
            insertColumn(destination_pos);
            insertRow(destination_pos);
            setHorizontalHeaderItem(destination_pos, new QTableWidgetItem{QString::number(destination_mac_address, 16)});
            setVerticalHeaderItem(destination_pos, new QTableWidgetItem{QString::number(destination_mac_address, 16)});
        }

        else
            destination_pos = node_pos[destination_mac_address];

        NetworkStatsTableItem *in_item = (NetworkStatsTableItem *) item(destination_pos, source_pos);
        if(!in_item)
        {
            in_item = new NetworkStatsTableItem{};
            setItem(destination_pos, source_pos, in_item);
        }

        in_item->setReceived(in);

        NetworkStatsTableItem *out_item = (NetworkStatsTableItem *) item(source_pos, destination_pos);
        if(!out_item)
        {
            out_item = new NetworkStatsTableItem{};
            setItem(source_pos, destination_pos, out_item);
        }

        out_item->setSent(out);
    }

    resizeColumnsToContents();
    resizeRowsToContents();
}

NetworkStatsTableItem::NetworkStatsTableItem()
    :QTableWidgetItem{}
    ,received{}
    ,sent{}
{
    updateText();
}

void NetworkStatsTableItem::setReceived(quint16 _received)
{
    received = _received;
    updateText();
}

void NetworkStatsTableItem::setSent(quint16 _sent)
{
    sent = _sent;
    updateText();
}

void NetworkStatsTableItem::updateText()
{
    setText(QString::asprintf("%u/%u", received, sent));
    setToolTip(QString::asprintf("%3.2f%%", 100.0 * (sent ? (double) received / sent : 1.0)));
}
