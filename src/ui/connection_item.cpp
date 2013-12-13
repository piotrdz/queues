#include "ui/connection_item.hpp"

#include "ui/station_item.hpp"

#include <QDebug>
#include <QPainter>

#include <cmath>

namespace
{
    const qreal PEN_WIDTH = 1.0;
    const qreal ARROW_SIZE = 15.0;
    const qreal LABEL_OFFSET = 8.0;
}


ConnectionItem::ConnectionItem(StationItem* source, StationItem* destination)
 : m_source(source)
 , m_destination(destination)
 , m_weight(1)
{
    m_weightFont = QFont("Sans Serif", 10);

    if (m_source != nullptr)
    {
        m_source->addConnection(this);
    }

    if (m_destination != nullptr)
    {
        m_destination->addConnection(this);
    }

    adjust();
}

ConnectionItem::~ConnectionItem()
{
    if (m_source != nullptr)
    {
        m_source->removeConnection(this);
        m_source = nullptr;
    }

    if (m_destination != nullptr)
    {
        m_destination->removeConnection(this);
        m_destination = nullptr;
    }
}

StationItem* ConnectionItem::getSource()
{
    return m_source;
}

StationItem* ConnectionItem::getDestination()
{
    return m_destination;
}

void ConnectionItem::setSource(StationItem* source)
{
    prepareGeometryChange();

    if (m_source != nullptr)
    {
        m_source->removeConnection(this);
    }

    m_source = source;

    if (m_source != nullptr)
    {
        m_source->addConnection(this);
    }
}

void ConnectionItem::setDestination(StationItem* destination)
{
    prepareGeometryChange();

    if (m_destination != nullptr)
    {
        m_destination->removeConnection(this);
    }

    m_destination = destination;

    if (m_destination != nullptr)
    {
        m_destination->removeConnection(this);
    }
}

void ConnectionItem::updateWeight(int weight)
{
    prepareGeometryChange();

    m_weight = weight;
}

void ConnectionItem::adjust()
{
    if (m_source == nullptr || m_destination == nullptr)
    {
        return;
    }

    QLineF line(mapFromItem(m_source, 0, 0), mapFromItem(m_destination, 0, 0));

    prepareGeometryChange();

    m_sourcePoint = line.p1();
    m_destinationPoint = line.p2();
}

QRectF ConnectionItem::boundingRect() const
{
    if (m_source == nullptr || m_destination == nullptr)
    {
        return QRectF();
    }

    qreal extra = (PEN_WIDTH + ARROW_SIZE) / 2.0;

    QRectF weightLabelRect = computeWeightLabelRect();

    return QRectF(m_sourcePoint, QSizeF(m_destinationPoint.x() - m_sourcePoint.x(),
                                        m_destinationPoint.y() - m_sourcePoint.y()))
        .normalized()
        .adjusted(-extra, -extra, extra, extra)
        .united(weightLabelRect);
}

QPointF ConnectionItem::computeSourcePoint() const
{
    QPolygonF sourceRect = mapFromItem(m_source, m_source->getBaseRect());

    QLineF connectionLine(m_sourcePoint, m_destinationPoint);

    for (int i = 0; i < sourceRect.size() - 1; ++i)
    {
        QLineF boundLine(sourceRect.at(i), sourceRect.at(i+1));
        QPointF intersectionPoint;
        QLineF::IntersectType intersectType = connectionLine.intersect(boundLine, &intersectionPoint);
        if (intersectType == QLineF::BoundedIntersection)
        {
            connectionLine = QLineF(intersectionPoint, m_destinationPoint);
        }
    }

    return connectionLine.p1();
}

QPointF ConnectionItem::computeDestinationPoint() const
{
    QPolygonF destintionRect = mapFromItem(m_destination, m_destination->getBaseRect());

    QLineF connectionLine(m_sourcePoint, m_destinationPoint);

    for (int i = 0; i < destintionRect.size() - 1; ++i)
    {
        QLineF boundLine(destintionRect.at(i), destintionRect.at(i+1));
        QPointF intersectionPoint;
        QLineF::IntersectType intersectType = connectionLine.intersect(boundLine, &intersectionPoint);
        if (intersectType == QLineF::BoundedIntersection)
        {
            connectionLine = QLineF(m_sourcePoint, intersectionPoint);
        }
    }

    return connectionLine.p2();
}

QRectF ConnectionItem::computeWeightLabelRect() const
{
    QLineF connectionLine(m_sourcePoint, m_destinationPoint);

    QFontMetricsF metrics(m_weightFont);

    QRectF textRect = metrics.boundingRect(QString().setNum(m_weight));

    qreal dx = std::fabs(connectionLine.dx());
    qreal dy = std::fabs(connectionLine.dy());

    qreal tw2 = textRect.width() / 2.0;
    qreal th2 = textRect.height() / 2.0;

    if (dx > dy && dx > tw2)
    {
        qreal deltaT = tw2 / dx;
        QPointF p1 = connectionLine.pointAt(0.5 - deltaT);
        QPointF p2 = connectionLine.pointAt(0.5 + deltaT);

        textRect.moveBottomRight(QPointF(std::min(p1.x(), p2.x()), std::min(p1.y(), p2.y()) - LABEL_OFFSET));
    }
    else if (dx <= dy && dy > th2)
    {
        qreal deltaT = th2 / dy;
        QPointF p1 = connectionLine.pointAt(0.5 - deltaT);
        QPointF p2 = connectionLine.pointAt(0.5 + deltaT);

        textRect.moveBottomRight(QPointF(std::min(p1.x(), p2.x()) + LABEL_OFFSET, std::min(p1.y(), p2.y())));
    }
    else
    {
        textRect.moveBottomRight(connectionLine.pointAt(0.5));
    }

    return textRect;
}

void ConnectionItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    if (m_source == nullptr || m_destination == nullptr)
    {
        return;
    }

    QPointF sourcePoint = computeSourcePoint();
    QPointF destinationPoint = computeDestinationPoint();

    QLineF line(sourcePoint, destinationPoint);
    if (qFuzzyCompare(line.length(), 0.0))
    {
        return;
    }

    painter->setPen(QPen(Qt::black, PEN_WIDTH, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->drawLine(line);

    double angle = std::atan2(line.dx(), line.dy());

    QPointF destArrowP1 = destinationPoint - QPointF( sin(angle - M_PI / 8.0) * ARROW_SIZE,
                                                      cos(angle - M_PI / 8.0) * ARROW_SIZE);
    QPointF destArrowP2 = destinationPoint - QPointF( sin(angle + M_PI / 8.0) * ARROW_SIZE,
                                                      cos(angle + M_PI / 8.0) * ARROW_SIZE);

    painter->setBrush(Qt::black);
    painter->drawPolygon(QPolygonF() << line.p2() << destArrowP1 << destArrowP2);

    QRectF weightLabelRect = computeWeightLabelRect();

    painter->setFont(m_weightFont);
    painter->drawText(weightLabelRect, Qt::AlignCenter, QString().setNum(m_weight));
}
