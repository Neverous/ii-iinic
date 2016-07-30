#ifndef NETWORKVISUALIZATION_H
#define NETWORKVISUALIZATION_H

#include <QGraphicsScene>
#include <QMap>

#include "networknode.h"
#include "networkedge.h"

class NetworkVisualization: public QGraphicsScene
{
    Q_OBJECT

public:
    static constexpr int    FPS        = 30;
    static constexpr int    MAX_RSSI   = 255;
    static constexpr int    NODE_SIZE  = 50;
    static constexpr int    SPEED      = 1;
    static constexpr qreal  PULL_CONSTANT = 0.01;
    static constexpr qreal  PUSH_CONSTANT = 50.0;

private:
    int								timer_id;
    QMap<quint16, NetworkNode *>	nodes;
    quint16                         root_mac_address;

public:
    NetworkVisualization();
    void clear();

    void update_root(quint16 mac_address);
    void update_node(quint16 mac_address, const QList<std::tuple<quint16, quint8, quint8>> &neighbours);
    void item_moved();

protected:
    void timerEvent(QTimerEvent *) Q_DECL_OVERRIDE;
};

#endif // NETWORKVISUALIZATION_H
