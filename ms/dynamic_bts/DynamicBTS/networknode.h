#ifndef NETWORKNODE_H
#define NETWORKNODE_H

#include <QGraphicsEllipseItem>
#include <QList>

class NetworkEdge;
class NetworkVisualization;

class NetworkNode: public QGraphicsEllipseItem
{
	friend class NetworkEdge;
    friend class NetworkVisualization;

private:
	QList<NetworkEdge *>	edges;
	quint16					mac_address;
	quint8					flags;
	QPointF					new_position;

public:
	NetworkNode(quint16 _mac_address, quint8 _flags=0);

	const QList<NetworkEdge *> &get_edges() const;

	quint16 get_mac_address() const;

	void calculate_forces();
	bool move();

protected:
	void add_edge(NetworkEdge *edge);
	void remove_edge(NetworkEdge *edge);

	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) Q_DECL_OVERRIDE;
	QVariant itemChange(GraphicsItemChange change, const QVariant &value) Q_DECL_OVERRIDE;
};

#endif // NETWORKNODE_H
