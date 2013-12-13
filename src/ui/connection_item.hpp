#pragma once

#include <QFont>
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

    void updateWeight(int weight);

    void adjust();

    virtual QRectF boundingRect() const;

    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event);

private:
    QPointF computeSourcePoint() const;
    QPointF computeDestinationPoint() const;
    QRectF computeWeightLabelRect() const;

    StationItem* m_source;
    StationItem* m_destination;
    QPointF m_sourcePoint;
    QPointF m_destinationPoint;
    int m_weight;
    QFont m_weightFont;
};
