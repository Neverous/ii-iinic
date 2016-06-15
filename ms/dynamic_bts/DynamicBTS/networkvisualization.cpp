#include "networkvisualization.h"

NetworkVisualization::NetworkVisualization()
	:QGraphicsScene{}
	,timer_id{}
    ,nodes{}
{
    item_moved();
}

void NetworkVisualization::clear()
{
    QGraphicsScene::clear();
    nodes.clear();
}

void NetworkVisualization::update_node(quint16 mac_address, const QList<std::tuple<quint16, quint8, quint8> > &neighbours)
{
	NetworkNode *&node = nodes[mac_address];
	if(!node)
		addItem(node = new NetworkNode{mac_address});

	QMap<quint16, NetworkEdge *> edges;
	for(const auto &edge: node->get_edges())
	{
		if(edge->get_source() == node)
			edges[edge->get_destination()->get_mac_address()] = edge;

		else
			edges[edge->get_source()->get_mac_address()] = edge;
	}

	for(const auto &neighbour: neighbours)
	{
		quint16 neighbour_mac_address;
        quint8 neighbour_rssi;
		quint8 neighbour_ttl;
		std::tie(neighbour_mac_address, neighbour_rssi, neighbour_ttl) = neighbour;

		if(!nodes.count(neighbour_mac_address))
			addItem(nodes[neighbour_mac_address] = new NetworkNode(neighbour_mac_address));

		if(edges.count(neighbour_mac_address))
		{
			auto &edge = edges[neighbour_mac_address];
            edge->rssi = (edge->rssi + neighbour_rssi) / 2;
            edge->ttl = qMax(edge->ttl, neighbour_ttl);
            edge->adjust();
			edges.remove(neighbour_mac_address);
		}

		else
		{
			addItem(new NetworkEdge{node, nodes[neighbour_mac_address], neighbour_rssi, neighbour_ttl});
		}
	}

	for(auto &left: edges.values())
	{
        removeItem(left);
        left->get_source()->remove_edge(left);
        left->get_destination()->remove_edge(left);
		delete left;
	}
}

void NetworkVisualization::item_moved()
{
	if(!timer_id)
		timer_id = startTimer(1000 / FPS);
}

void NetworkVisualization::timerEvent(QTimerEvent *)
{
    static int counter = 0;
    ++ counter;
    if(counter == FPS)
    {
        counter = 0;
        QList<NetworkEdge *> dead;
        for(const auto &item: items())
        {
            NetworkEdge *edge = qgraphicsitem_cast<NetworkEdge *>(item);
            if(edge && !-- edge->ttl)
                dead.append(edge);
        }

        for(auto &edge: dead)
        {
            removeItem(edge);
            edge->get_source()->remove_edge(edge);
            edge->get_destination()->remove_edge(edge);
            delete edge;
        }

        update();
    }

	QList<NetworkNode *> nodes;
	for(const auto &item: items())
	{
		NetworkNode *node = qgraphicsitem_cast<NetworkNode *>(item);
		if(node)
			nodes.append(node);
	}

	for(auto &node: nodes)
		node->calculate_forces();

	bool items_moved = false;
	for(auto &node: nodes)
		items_moved |= node->move();

	QPointF middle;
	for(auto &node: nodes)
		middle += node->pos();

	middle /= nodes.size();

	for(auto &node: nodes)
        node->moveBy(-middle.x(), -middle.y());
}
