#pragma once

#include "engine/station.hpp"

#include <QFont>
#include <QGraphicsItem>
#include <QSet>

class ConnectionItem;

class StationItem : public QGraphicsItem
{
public:
    StationItem();
    virtual ~StationItem();

    void addConnection(ConnectionItem* edge);
    void removeConnection(ConnectionItem* edge);

    void updateInfo(const Station& stationInfo);

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

    virtual QRectF boundingRect() const override;

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    QSizeF getBaseSize() const;
    QSizeF getLabelSize() const;
    QSizeF getTaskSize() const;
    QString getLabel() const;

    Station m_stationInfo;
    QFont m_labelFont;
    QFont m_taskFont;
    QSet<ConnectionItem*> m_connections;
};
