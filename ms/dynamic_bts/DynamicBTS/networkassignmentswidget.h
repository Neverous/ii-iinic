#ifndef NETWORKASSIGNMENTSWIDGET_H
#define NETWORKASSIGNMENTSWIDGET_H

#include <QMap>
#include <QTableWidget>


class NetworkAssignmentsWidget: public QTableWidget
{
private:
    QMap<quint16, int>  node_pos;

public:
    explicit NetworkAssignmentsWidget(QWidget *parent=nullptr);
    void clear();

    void update_assignments(const QList<std::tuple<quint16, quint8, quint8, quint16>> &assignments);
};

#endif // NETWORKASSIGNMENTSWIDGET_H
