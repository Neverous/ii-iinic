#ifndef NETWORKSTATSTABLEWIDGET_H
#define NETWORKSTATSTABLEWIDGET_H

#include <QMap>
#include <QTableWidget>

class NetworkStatsTableWidget: public QTableWidget
{
private:
    QMap<quint16, int> node_pos;
public:
    explicit NetworkStatsTableWidget(QWidget *parent=nullptr);

    void clear();

    void update_node(quint16 source_mac_address, const QList<std::tuple<quint16, quint16, quint16> > &stats);
};

class NetworkStatsTableItem: public QTableWidgetItem
{
private:
    quint16 received;
    quint16 sent;

public:
    NetworkStatsTableItem();

    void setReceived(quint16 _received);
    void setSent(quint16 _sent);

private:
    void updateText();
};

#endif // NETWORKSTATSTABLEWIDGET_H
