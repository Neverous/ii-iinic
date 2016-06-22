#include "qgraphicsfixedview.h"

QGraphicsFixedView::QGraphicsFixedView(QWidget *parent)
	:QGraphicsView{parent}
{

}

void QGraphicsFixedView::setScene(QGraphicsScene *scene)
{
	auto _width = width();
	auto _height = height();
	scene->setSceneRect(-_width / 2 + 20, -_height / 2 + 20, _width - 20, _height - 20);
	QGraphicsView::setScene(scene);
}

void QGraphicsFixedView::resizeEvent(QResizeEvent *event)
{
    auto _width = width();
    auto _height = height();
    scene()->setSceneRect(-_width / 2 + 20, -_height / 2 + 20, _width - 20, _height - 20);
    QGraphicsView::resizeEvent(event);
}
