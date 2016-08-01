#ifndef LATENCYLOGWIDGET_H
#define LATENCYLOGWIDGET_H

#include <QMap>
#include <QTableWidget>

class LatencyLogWidget: public QTableWidget
{
private:
    QMap<quint16, int> node_pos;

public:
    explicit LatencyLogWidget(QWidget *parent=nullptr);
    void clear();

    void update_node(quint16 mac_address, quint16 latency);

private:
    void check_row();
};

#endif // LATENCYLOGWIDGET_H
