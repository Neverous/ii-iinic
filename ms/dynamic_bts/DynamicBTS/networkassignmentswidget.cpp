#include "networkassignmentswidget.h"

NetworkAssignmentsWidget::NetworkAssignmentsWidget(QWidget *parent)
    :QTableWidget{parent}
    ,node_pos{}
{

}

void NetworkAssignmentsWidget::clear()
{
    node_pos.clear();
    setColumnCount(0);
}

void NetworkAssignmentsWidget::update_assignments(const QList<std::tuple<quint16, quint8, quint8, quint16> > &assignments)
{
    for(auto &assignment: assignments)
    {
        quint16 mac_address;
        quint8 priority;
        quint8 ttl;
        quint16 slotmask;
        std::tie(mac_address, priority, ttl, slotmask) = assignment;

        int column;
        if(!node_pos.count(mac_address))
        {
            column = node_pos[mac_address] = columnCount();
            insertColumn(column);
            setHorizontalHeaderItem(column, new QTableWidgetItem{QString::number(mac_address, 16)});
        }
        else
            column = node_pos[mac_address];

        horizontalHeaderItem(column)->setToolTip(QString::number(priority));
        for(int s = 0; s < 16; ++ s)
        {
            auto cell = item(s, column);
            if(!cell)
            {
                cell = new QTableWidgetItem{};
                setItem(s, column, cell);
            }

            else

            if((slotmask & (1 << s)) && ttl)
            {
                cell->setBackgroundColor(Qt::green);
                cell->setForeground({Qt::black});
                cell->setText(QString::number(ttl));
            }

            else
            {
                cell->setBackgroundColor(Qt::transparent);
                cell->setForeground({Qt::transparent});
                cell->setText(QString::number(0));
            }
        }
    }

    resizeColumnsToContents();
}
