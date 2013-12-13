#include "ui/connection_item.hpp"

#include "ui/station_item.hpp"

#include <QPainter>

#include <cmath>

namespace
{
    const qreal PEN_WIDTH = 1.0;
    const qreal ARROW_SIZE = 15.0;
    const qreal LABEL_OFFSET = 8.0;
    const qreal SELECTION_MARKER_SIZE = 5.0;
    const qreal SELECTION_ZONE_WIDTH = 10.0;
}


ConnectionItem::ConnectionItem(StationItem* source, StationItem* destination, bool moveMode)
 : m_source(source)
 , m_destination(destination)
 , m_weight(1)
 , m_moveMode(moveMode)
{
    setFlag(QGraphicsItem::ItemIsSelectable);
    setAcceptedMouseButtons(Qt::LeftButton);

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
    if (m_source == nullptr || (!m_moveMode && m_destination == nullptr))
    {
        return;
    }

    QPointF destinationPoint = m_moveMode ? m_moveDestinationPoint : mapFromItem(m_destination, 0, 0);

    QLineF line(mapFromItem(m_source, 0, 0), destinationPoint);

    prepareGeometryChange();

    m_sourcePoint = line.p1();
    m_destinationPoint = line.p2();
}

void ConnectionItem::setMovePoint(const QPointF& moveDestiationPoint)
{
    prepareGeometryChange();

    m_moveDestinationPoint = mapFromParent(moveDestiationPoint);

    adjust();
}

QRectF ConnectionItem::boundingRect() const
{
    if (m_source == nullptr || (!m_moveMode && m_destination == nullptr))
    {
        return QRectF();
    }

    qreal extra = (PEN_WIDTH + ARROW_SIZE) / 2.0;

    QRectF weightLabelRect = getWeightLabelRect();

    QPointF destinationPoint = getDestinationPoint();

    return QRectF(m_sourcePoint, QSizeF(destinationPoint.x() - m_sourcePoint.x(),
                                        destinationPoint.y() - m_sourcePoint.y()))
        .normalized()
        .adjusted(-extra, -extra, extra, extra)
        .united(weightLabelRect);
}

QPainterPath ConnectionItem::shape() const
{
    QPointF sourcePoint = getDrawSourcePoint();
    QPointF destinationPoint = getDrawDestinationPoint();

    QLineF line(sourcePoint, destinationPoint);

    QLineF nor = line.normalVector();
    QPointF deltaNor = nor.p2() - nor.p1();
    deltaNor /= nor.length();
    deltaNor *= SELECTION_ZONE_WIDTH;

    QPolygonF polygon;
    polygon << (line.p1() + deltaNor) << (line.p1() - deltaNor) << (line.p2() - deltaNor) << (line.p2() + deltaNor);

    QPainterPath path;
    path.addPolygon(polygon);

    return path;
}

QPointF ConnectionItem::getDestinationPoint() const
{
    return m_moveMode ? m_moveDestinationPoint : m_destinationPoint;
}

QPointF ConnectionItem::getDrawSourcePoint() const
{
    QPolygonF sourceRect = mapFromItem(m_source, m_source->getBaseRect());

    QLineF connectionLine(m_sourcePoint, getDestinationPoint());

    for (int i = 0; i < sourceRect.size() - 1; ++i)
    {
        QLineF boundLine(sourceRect.at(i), sourceRect.at(i+1));
        QPointF intersectionPoint;
        QLineF::IntersectType intersectType = connectionLine.intersect(boundLine, &intersectionPoint);
        if (intersectType == QLineF::BoundedIntersection)
        {
            connectionLine = QLineF(intersectionPoint, getDestinationPoint());
        }
    }

    return connectionLine.p1();
}

QPointF ConnectionItem::getDrawDestinationPoint() const
{
    if (m_moveMode)
    {
        return m_destinationPoint;
    }

    QPolygonF destinationRect = mapFromItem(m_destination, m_destination->getBaseRect());

    QLineF connectionLine(m_sourcePoint, getDestinationPoint());

    for (int i = 0; i < destinationRect.size() - 1; ++i)
    {
        QLineF boundLine(destinationRect.at(i), destinationRect.at(i+1));
        QPointF intersectionPoint;
        QLineF::IntersectType intersectType = connectionLine.intersect(boundLine, &intersectionPoint);
        if (intersectType == QLineF::BoundedIntersection)
        {
            connectionLine = QLineF(m_sourcePoint, intersectionPoint);
        }
    }

    return connectionLine.p2();
}

QRectF ConnectionItem::getWeightLabelRect() const
{
    QLineF connectionLine(m_sourcePoint, getDestinationPoint());

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
    if (m_source == nullptr || (!m_moveMode && m_destination == nullptr))
    {
        return;
    }

    QPointF sourcePoint = getDrawSourcePoint();
    QPointF destinationPoint = getDrawDestinationPoint();

    QLineF line(sourcePoint, destinationPoint);
    if (qFuzzyCompare(line.length(), 0.0))
    {
        return;
    }

    if (isSelected())
    {
        QSizeF markerSize(SELECTION_MARKER_SIZE, SELECTION_MARKER_SIZE);

        painter->setPen(QPen(Qt::black));
        painter->setBrush(QBrush(Qt::black));

        painter->drawRect(QRectF(line.p1(), markerSize));
        painter->drawRect(QRectF(line.p2(), markerSize));
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

    QRectF weightLabelRect = getWeightLabelRect();

    painter->setFont(m_weightFont);
    painter->drawText(weightLabelRect, Qt::AlignCenter, QString().setNum(m_weight));
}
