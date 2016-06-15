#include <QPainter>

#include "networkvisualization.h"
#include "networkedge.h"
#include "networknode.h"

NetworkEdge::NetworkEdge(NetworkNode *_source, NetworkNode *_destination, quint16 _rssi, quint8 _ttl)
	:source{_source}
	,destination{_destination}
	,rssi{_rssi}
	,ttl{_ttl}
{
	setAcceptedMouseButtons(0);
    setFlag(ItemSendsGeometryChanges);
	setPen({Qt::white});
	source->add_edge(this);
	destination->add_edge(this);
	adjust();
}

NetworkNode *NetworkEdge::get_source() const
{
	return source;
}

NetworkNode *NetworkEdge::get_destination() const
{
	return destination;
}

quint16 NetworkEdge::get_rssi() const
{
	return rssi;
}

void NetworkEdge::adjust()
{
	if(!source || !destination)
		return;

	QLineF line{mapFromItem(source, 0, 0), mapFromItem(destination, 0, 0)};
	qreal length = line.length();

	prepareGeometryChange();
    if(length > NetworkVisualization::NODE_SIZE)
	{
        QPointF offset{(line.dx() * NetworkVisualization::NODE_SIZE / 2) / length, (line.dy() * NetworkVisualization::NODE_SIZE / 2) / length};
		setLine(QLineF{line.p1() + offset, line.p2() - offset});
	}

	else
	{
		setLine(0, 0, 0, 0);
	}
}

QRectF NetworkEdge::boundingRect() const
{
    return QGraphicsLineItem::boundingRect().adjusted(-NetworkVisualization::NODE_SIZE / 2, -NetworkVisualization::NODE_SIZE / 2, NetworkVisualization::NODE_SIZE / 2, NetworkVisualization::NODE_SIZE / 2);
}

void NetworkEdge::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	QGraphicsLineItem::paint(painter, option, widget);
    painter->drawText(QGraphicsLineItem::boundingRect().adjusted(-NetworkVisualization::NODE_SIZE / 2, -NetworkVisualization::NODE_SIZE / 2, NetworkVisualization::NODE_SIZE / 2, 0), Qt::AlignHCenter | Qt::AlignVCenter, QString::number(rssi));
    painter->drawText(QGraphicsLineItem::boundingRect().adjusted(-NetworkVisualization::NODE_SIZE / 2, 0, NetworkVisualization::NODE_SIZE / 2, NetworkVisualization::NODE_SIZE / 2), Qt::AlignHCenter | Qt::AlignVCenter, QString::number(ttl));
}
