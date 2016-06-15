#ifndef NETWORKEDGE_H
#define NETWORKEDGE_H

#include <QGraphicsLineItem>
#include <QtMath>

class NetworkNode;
class NetworkVisualization;

class NetworkEdge: public QGraphicsLineItem
{
	friend class NetworkEdge;
    friend class NetworkVisualization;

private:
	NetworkNode *source;
	NetworkNode *destination;
	quint16		rssi;
	quint8		ttl;

public:
	NetworkEdge(NetworkNode *_source, NetworkNode *_destination, quint16 _rssi=0, quint8 _ttl=255);

	void disconnect();

	NetworkNode *get_source() const;
	NetworkNode *get_destination() const;
	quint16 get_rssi() const;

	void adjust();

protected:
	QRectF boundingRect() const Q_DECL_OVERRIDE;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) Q_DECL_OVERRIDE;
};

#endif // NETWORKEDGE_H
