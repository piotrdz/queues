#include "ui/station_item.hpp"

#include "ui/connection_item.hpp"

#include <QDebug>
#include <QGraphicsSceneEvent>
#include <QPainter>


namespace
{
    const qreal SELECTION_MARKER_SIZE = 5.0;
    const qreal LABEL_RECT_SPACING = 2.0;
    const qreal TASK_RECT_SPACING = 2.0;
    const qreal QUEUE_SPACING = 8.0;
    const qreal PROCESSOR_SPACING = 6.0;
}


StationItem::StationItem()
{
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);

    m_labelFont = QFont("Sans Serif", 12, QFont::Bold);
    m_taskFont = QFont("Sans Serif", 10);
}

StationItem::~StationItem()
{
    auto edgesCopy = m_connections;

    for (ConnectionItem* edge : edgesCopy)
    {
        if (edge->getSource() == this)
        {
            edge->setSource(nullptr);
        }

        if (edge->getDestination() == this)
        {
            edge->setDestination(nullptr);
        }
    }

    // edges shoula all remove themselves
    Q_ASSERT(m_connections.isEmpty());
}

void StationItem::updateInfo(const Station& stationInfo)
{
    prepareGeometryChange();
    m_stationInfo = stationInfo;
}

void StationItem::addConnection(ConnectionItem* edge)
{
    m_connections.insert(edge);
}

void StationItem::removeConnection(ConnectionItem* edge)
{
    m_connections.remove(edge);
}

QVariant StationItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
{
    if (change == QGraphicsItem::ItemPositionHasChanged)
    {
        for (ConnectionItem* edge : m_connections)
        {
            edge->adjust();
        }
    }

    return QGraphicsItem::itemChange(change, value);
}

QRectF StationItem::boundingRect() const
{
    QRectF boundingRect = getBaseRect();
    boundingRect.adjust(-SELECTION_MARKER_SIZE, -SELECTION_MARKER_SIZE,
                        SELECTION_MARKER_SIZE, SELECTION_MARKER_SIZE);
    return boundingRect;
}

QRectF StationItem::getBaseRect() const
{
    QSizeF baseSize = getBaseSize();

    QRectF baseRect(QPointF(0, 0), baseSize);
    baseRect.translate(-baseSize.width() / 2.0, -baseSize.height() / 2.0);

    return baseRect;
}

QSizeF StationItem::getBaseSize() const
{
    QSizeF labelSize = getLabelSize();

    QSizeF taskSize = getTaskSize();

    qreal width = QUEUE_SPACING + m_stationInfo.queueLength * taskSize.width() + QUEUE_SPACING +
                  PROCESSOR_SPACING + taskSize.width() + PROCESSOR_SPACING;

    qreal queueAreaHeight = labelSize.height() + QUEUE_SPACING + taskSize.height() + QUEUE_SPACING;
    qreal processorAreaHeight = PROCESSOR_SPACING + (PROCESSOR_SPACING + taskSize.height()) * m_stationInfo.processorCount;

    qreal height = std::max(queueAreaHeight, processorAreaHeight);

    return QSizeF(width, height);
}

QSizeF StationItem::getLabelSize() const
{
    QFontMetricsF metrics(m_labelFont);
    return metrics.size(0, getLabel()) + QSizeF(2*LABEL_RECT_SPACING, 2*LABEL_RECT_SPACING);
}

QSizeF StationItem::getTaskSize() const
{
    QFontMetricsF metrics(m_taskFont);

    return metrics.size(0, "99") + QSizeF(2*TASK_RECT_SPACING, 2*TASK_RECT_SPACING);
}

QString StationItem::getLabel() const
{
    QString label;
    label.setNum(m_stationInfo.id);
    return label;
}

void StationItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    QSizeF baseSize = getBaseSize();
    QSizeF labelSize = getLabelSize();
    QSizeF taskSize = getTaskSize();

    painter->translate(-baseSize.width() / 2.0 - SELECTION_MARKER_SIZE, -baseSize.height() / 2.0 - SELECTION_MARKER_SIZE);

    // selection markers

    if (isSelected())
    {
        QSizeF markerSize(SELECTION_MARKER_SIZE, SELECTION_MARKER_SIZE);

        painter->setPen(QPen(Qt::black));
        painter->setBrush(QBrush(Qt::black));

        painter->drawRect(QRectF(QPointF(0.0, 0.0), markerSize));
        painter->drawRect(QRectF(QPointF(SELECTION_MARKER_SIZE + baseSize.width(), 0.0), markerSize));
        painter->drawRect(QRectF(QPointF(0.0, SELECTION_MARKER_SIZE + baseSize.height()), markerSize));
        painter->drawRect(QRectF(QPointF(SELECTION_MARKER_SIZE + baseSize.width(),
                                         SELECTION_MARKER_SIZE + baseSize.height()),
                                 markerSize));
    }

    painter->translate(SELECTION_MARKER_SIZE, SELECTION_MARKER_SIZE);

    // bounding rect

    painter->setBrush(QBrush(Qt::NoBrush));
    painter->setPen(QPen(Qt::black));

    painter->drawRect(QRectF(QPointF(0.0, 0.0), baseSize));

    // label

    painter->setBrush(QBrush(Qt::NoBrush));
    painter->setPen(QPen(Qt::darkBlue));
    painter->setFont(m_labelFont);

    QRectF labelRect(QPointF(0.0, 0.0), labelSize);
    painter->drawRect(labelRect);
    painter->drawText(labelRect, Qt::AlignCenter, getLabel());

    // queue

    painter->setFont(m_taskFont);

    qreal queueAreaHeight = labelSize.height() + QUEUE_SPACING + taskSize.height() + QUEUE_SPACING;
    qreal processorAreaHeight = PROCESSOR_SPACING + (PROCESSOR_SPACING + taskSize.height()) * m_stationInfo.processorCount;

    qreal queueHeight = 0.0;

    if (queueAreaHeight > processorAreaHeight)
    {
        queueHeight = labelSize.height() + QUEUE_SPACING;
    }
    else
    {
        queueHeight = std::max(processorAreaHeight / 2.0 - taskSize.height() / 2.0,
                               labelSize.height() + QUEUE_SPACING);
    }

    QPointF queuePos(QUEUE_SPACING, queueHeight);

    painter->setPen(QPen(Qt::black));

    for (int i = 0; i < m_stationInfo.queueLength; ++i)
    {
        QRectF queueTaskRect(queuePos, taskSize);
        painter->drawRect(queueTaskRect);
        painter->drawText(queueTaskRect, Qt::AlignCenter, "X");

        queuePos += QPointF(taskSize.width(), 0.0);
    }

    // processors

    qreal queueAreaWidth = QUEUE_SPACING + m_stationInfo.queueLength * taskSize.width() + QUEUE_SPACING;

    QPointF processorPos(queueAreaWidth + PROCESSOR_SPACING, PROCESSOR_SPACING);

    for (int i = 0; i < m_stationInfo.processorCount; ++i)
    {
        QRectF processorTaskRect(processorPos, taskSize);
        painter->drawRect(processorTaskRect);
        painter->drawText(processorTaskRect, Qt::AlignCenter, "Y");

        processorPos += QPointF(0.0, taskSize.height() + PROCESSOR_SPACING);
    }
}
