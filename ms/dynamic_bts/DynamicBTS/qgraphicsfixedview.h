#ifndef QGRAPHICSFIXEDVIEW_H
#define QGRAPHICSFIXEDVIEW_H

#include <QGraphicsView>

class QGraphicsFixedView: public QGraphicsView
{
public:
    QGraphicsFixedView(QWidget *parent=nullptr);

    void setScene(QGraphicsScene *scene);

protected:
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
};

#endif // QGRAPHICSFIXEDVIEW_H
