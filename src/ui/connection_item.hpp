#pragma once

#include <QGraphicsItem>

class StationItem;

class ConnectionItem : public QGraphicsItem
{
public:
    ConnectionItem(StationItem* source, StationItem* destination);
    virtual ~ConnectionItem();

    StationItem* getSource();
    StationItem* getDestination();

    void setSource(StationItem* source);
    void setDestination(StationItem* destination);

    void adjust();

    virtual QRectF boundingRect() const;

    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

private:
    StationItem* m_source;
    StationItem* m_destination;
    QPointF m_sourcePoint;
    QPointF m_destinationPoint;
};
