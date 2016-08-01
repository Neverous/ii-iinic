#include <QGraphicsScene>
#include <QPainter>
#include <QStyleOption>
#include <QtMath>

#include "networkedge.h"
#include "networknode.h"
#include "networkvisualization.h"

NetworkNode::NetworkNode(quint16 _mac_address, quint8 _flags)
    :QGraphicsEllipseItem{-NetworkVisualization::NODE_SIZE/2, -NetworkVisualization::NODE_SIZE/2, NetworkVisualization::NODE_SIZE, NetworkVisualization::NODE_SIZE}
    ,edges{}
    ,mac_address{_mac_address}
    ,flags{_flags}
    ,new_position{}
    ,backup_pen{}
    ,is_root{false}
{
    setFlag(ItemIsMovable);
    setFlag(ItemIsFocusable);
    setFlag(ItemSendsGeometryChanges);
    setCacheMode(DeviceCoordinateCache);
    setZValue(-1);
    setPen({Qt::white});
}

const QList<NetworkEdge *> &NetworkNode::get_edges() const
{
    return edges;
}

quint16 NetworkNode::get_mac_address() const
{
    return mac_address;
}

void NetworkNode::calculate_forces()
{
    if(!scene() || scene()->mouseGrabberItem() == this)
    {
        new_position = pos();
        return;
    }

    // Sum up all forces pushing this item away
    qreal x_velocity = 0;
    qreal y_velocity = 0;
    for(const auto &item: scene()->items())
    {
        NetworkNode *node = qgraphicsitem_cast<NetworkNode *>(item);
        if(!node)
            continue;

        if(node == this)
            continue;

        QPointF vec = mapToItem(node, 0, 0);
        qreal dx = vec.x();
        qreal dy = vec.y();

        if(qAbs(dx) < 0.1 && qAbs(dy) < 0.1)
        {
            dx = qrand() & 1 ? -1 : 1;
            dy = qrand() & 1 ? -1 : 1;
        }

        qreal dist  = qSqrt(dx * dx + dy * dy);
        qreal rel_dist = log(1. + dist);
        qreal force = NetworkVisualization::PUSH_CONSTANT / rel_dist / rel_dist;

        x_velocity += force * dx / dist;
        y_velocity += force * dy / dist;
    }

    // Subtract all forces pulling items together
    for(const auto &edge: edges)
    {
        QPointF vec;
        if(edge->get_source() == this)
            vec = mapToItem(edge->get_destination(), 0, 0);

        else
            vec = mapToItem(edge->get_source(), 0, 0);

        qreal dx = vec.x();
        qreal dy = vec.y();

        if(qAbs(dx) < 0.1 && qAbs(dy) < 0.1)
        {
            dx = qrand() & 1 ? -1 : 1;
            dy = qrand() & 1 ? -1 : 1;
        }

        qreal dist = qSqrt(dx * dx + dy * dy);
        qreal max_dist = qMin(scene()->height(), scene()->width());
        qreal ratio = 1.0 * (NetworkVisualization::MAX_RSSI - edge->get_rssi()) / NetworkVisualization::MAX_RSSI;
        qreal rel_dist = dist - NetworkVisualization::NODE_SIZE * 1.5 - max_dist * ratio * ratio;
        qreal force = NetworkVisualization::PULL_CONSTANT * rel_dist;

        x_velocity -= force * dx / dist;
        y_velocity -= force * dy / dist;
    }

    if(qAbs(x_velocity) < 0.1 && qAbs(y_velocity) < 0.1)
        x_velocity = y_velocity = 0;

    QRectF scene_rect = scene()->sceneRect();
    new_position = pos() + NetworkVisualization::SPEED * QPointF{x_velocity, y_velocity};
    new_position.setX(qMin(qMax(new_position.x(), scene_rect.left() + NetworkVisualization::NODE_SIZE), scene_rect.right() - NetworkVisualization::NODE_SIZE));
    new_position.setY(qMin(qMax(new_position.y(), scene_rect.top() + NetworkVisualization::NODE_SIZE), scene_rect.bottom() - NetworkVisualization::NODE_SIZE));
}

bool NetworkNode::move()
{
    if(new_position == pos())
        return false;

    setPos(new_position);
    return true;
}

void NetworkNode::setRoot(bool value)
{
    is_root = value;
    update();
}

void NetworkNode::add_edge(NetworkEdge *edge)
{
    edges.append(edge);
}

void NetworkNode::remove_edge(NetworkEdge *edge)
{
    edges.removeOne(edge);
}

void NetworkNode::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QGraphicsEllipseItem::paint(painter, option, widget);
    if(is_root)
        painter->setPen(QPen{Qt::red});

    painter->drawText(boundingRect(), Qt::AlignHCenter | Qt::AlignVCenter, QString::number(mac_address, 16));
}

void NetworkNode::focusInEvent(QFocusEvent *event)
{
    backup_pen = pen();
    setPen({Qt::gray});
    QGraphicsEllipseItem::focusInEvent(event);
}

void NetworkNode::focusOutEvent(QFocusEvent *event)
{
    setPen(backup_pen);
    QGraphicsEllipseItem::focusOutEvent(event);
}

QVariant NetworkNode::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if(change == ItemPositionHasChanged)
        for(auto &edge: edges)
            edge->adjust();

    return QGraphicsEllipseItem::itemChange(change, value);
}
