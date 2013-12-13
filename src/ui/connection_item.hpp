#pragma once

#include <QFont>
#include <QGraphicsItem>

class StationItem;

class ConnectionItem : public QGraphicsItem
{
public:
    ConnectionItem(StationItem* source, StationItem* destination, bool moveMode = false);
    virtual ~ConnectionItem();

    StationItem* getSource();
    StationItem* getDestination();

    void setSource(StationItem* source);
    void setDestination(StationItem* destination);

    void updateWeight(int weight);

    void adjust();

    void setMovePoint(const QPointF& moveDestiationPoint);

    virtual QRectF boundingRect() const override;
    virtual QPainterPath shape() const override;

    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    QPointF getDestinationPoint() const;
    QPointF getDrawSourcePoint() const;
    QPointF getDrawDestinationPoint() const;
    QRectF getWeightLabelRect() const;

    StationItem* m_source;
    StationItem* m_destination;
    QPointF m_sourcePoint;
    QPointF m_destinationPoint;
    int m_weight;
    QFont m_weightFont;
    bool m_moveMode;
    QPointF m_moveDestinationPoint;
};
