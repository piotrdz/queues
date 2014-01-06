#pragma once

#include "engine/event.hpp"
#include "engine/station.hpp"

#include <QFont>
#include <QGraphicsItem>
#include <QSet>

class ConnectionItem;

class StationItem : public QGraphicsItem
{
public:
    StationItem(const Station& station);
    virtual ~StationItem();

    void addConnection(ConnectionItem* edge);
    void removeConnection(ConnectionItem* edge);

    void updateParams(const StationParams& stationParams);
    int getId() const;

    void reset();
    void newEvent(Event event);

    QPolygonF getOutlineBasePolygon() const;

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

    virtual QRectF boundingRect() const override;

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    void paintSelectionMarkers(QPainter* painter, const QSizeF& baseSize);
    void paintInputOutputNode(QPainter* painter, const QSizeF& baseSize);
    void paintBoundingRect(QPainter* painter, const QSizeF& baseSize);
    void paintLabel(QPainter* painter, const QSizeF& labelSize);
    void paintProcessors(QPainter* painter, const QSizeF& queueTaskSize, const QSizeF& processorTaskSize);
    void paintQueue(QPainter* painter, const QSizeF& labelSize, const QSizeF& queueTaskSize, const QSizeF& processorTaskSize);

    QString getTaskText(int number);

private:
    QRectF getBaseRect() const;
    QSizeF getBaseSize() const;
    QSizeF getLabelSize() const;
    QSizeF getProcessorTaskSize() const;
    QSizeF getQueueTaskSize() const;
    QString getLabel() const;

    Station m_stationInfo;
    QFont m_labelFont;
    QFont m_taskFont;
    QSet<ConnectionItem*> m_connections;

    QList<int> m_tasksInQueue;
    QList<int> m_tasksInProcessors;
};
