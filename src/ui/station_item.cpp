#include "ui/station_item.hpp"

#include "ui/connection_item.hpp"

#include <QDebug>
#include <QGraphicsSceneEvent>
#include <QPainter>

#include <cmath>


namespace
{
    const qreal SELECTION_MARKER_SIZE = 5.0;
    const qreal LABEL_RECT_SPACING = 2.0;
    const qreal TASK_RECT_SPACING = 2.0;
    const qreal QUEUE_SPACING = 8.0;
    const qreal PROCESSOR_SPACING = 6.0;
    const qreal INPUT_OUTPUT_DIAMETER = 40.0;
    const int OUTLINE_VERTICES = 8;
}


StationItem::StationItem(const Station& station)
 : m_stationInfo(station)
{
    updateParams(station);

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

    Q_ASSERT(m_connections.isEmpty());
}

void StationItem::updateParams(const StationParams& stationParams)
{
    prepareGeometryChange();

    m_stationInfo.setParams(stationParams);

    reset();
}

int StationItem::getId() const
{
    return m_stationInfo.id;
}

void StationItem::reset()
{
    m_tasksInQueue.clear();
    for (int i = 0; i < m_stationInfo.queueLength; ++i)
    {
        m_tasksInQueue.append(EMPTY_TASK_ID);
    }

    m_tasksInProcessors.clear();
    for (int i = 0; i < m_stationInfo.processorCount; ++i)
    {
        m_tasksInProcessors.append(EMPTY_TASK_ID);
    }

    update();
}

void StationItem::newEvent(Event event)
{
    if (m_stationInfo.id == INPUT_STATION_ID ||
        m_stationInfo.id == OUTPUT_STATION_ID)
    {
        qDebug() << "ERROR: Event at input/output station";
        return;
    }

    if (event.type == EventType::TaskAddedToQueue)
    {
        if (m_stationInfo.queueLength == 0)
        {
            m_tasksInQueue.append(event.taskId);
        }
        else
        {
            bool addedOk = false;
            for (int& task : m_tasksInQueue)
            {
                if (task == EMPTY_TASK_ID)
                {
                    task = event.taskId;
                    addedOk = true;
                    break;
                }
            }
            if (!addedOk)
            {
                qDebug() << "ERROR: Task does not fit in queue: " << event.taskId;
            }
        }
    }
    else if (event.type == EventType::TaskStartedProcessing)
    {
        if (m_stationInfo.queueLength == 0)
        {
            bool removedOk = m_tasksInQueue.removeOne(event.taskId);
            if (!removedOk)
            {
                qDebug() << "ERROR: Task not in queue:" << event.taskId;
            }
        }
        else
        {
            bool removedOk = false;
            QList<int> newTasks;
            for (int task : m_tasksInQueue)
            {
                if (task == event.taskId)
                {
                    removedOk = true;
                }
                else
                {
                    newTasks.append(task);
                }
            }

            for (int i = 0; i < m_tasksInQueue.size(); ++i)
            {
                if (i < newTasks.size())
                {
                    m_tasksInQueue[i] = newTasks[i];
                }
                else
                {
                    m_tasksInQueue[i] = EMPTY_TASK_ID;
                }
            }

            if (!removedOk)
            {
                qDebug() << "ERROR: Task not in queue:" << event.taskId;
            }
        }

        bool addedOk = false;
        for (int& processorTask : m_tasksInProcessors)
        {
            if (processorTask == EMPTY_TASK_ID)
            {
                processorTask = event.taskId;
                addedOk = true;
                break;
            }
        }

        if (!addedOk)
        {
            qDebug() << "ERROR: Task could not be added to processor:" << event.taskId;
        }
    }
    else if (event.type == EventType::TaskEndedProcessing)
    {
        bool changedOk = false;
        for (int& processorTask : m_tasksInProcessors)
        {
            if (processorTask == event.taskId)
            {
                processorTask = -event.taskId;
                changedOk = true;
                break;
            }
        }

        if (!changedOk)
        {
            qDebug() << "ERROR: Task not found on processors: " << event.taskId;
        }
    }
    else if (event.type == EventType::MachineIsIdle)
    {
        bool changedOk = false;
        for (int& processorTask : m_tasksInProcessors)
        {
            if (processorTask == -event.taskId)
            {
                processorTask = EMPTY_TASK_ID;
                changedOk = true;
                break;
            }
        }

        if (!changedOk)
        {
            qDebug() << "ERROR: Task not found on processors: " << event.taskId;
        }
    }
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

QPolygonF StationItem::getOutlineBasePolygon() const
{
    if (m_stationInfo.id == INPUT_STATION_ID ||
        m_stationInfo.id == OUTPUT_STATION_ID)
    {
        QPolygonF circle;
        for (int i = 0; i <= OUTLINE_VERTICES; ++i)
        {
            circle.append(QPointF(std::sin(2.0 * i * M_PI / OUTLINE_VERTICES) * INPUT_OUTPUT_DIAMETER/2.0,
                                  std::cos(2.0 * i * M_PI / OUTLINE_VERTICES) * INPUT_OUTPUT_DIAMETER/2.0));
        }
        return circle;
    }

    return QPolygonF(getBaseRect());
}

QSizeF StationItem::getBaseSize() const
{
    if (m_stationInfo.id == INPUT_STATION_ID ||
        m_stationInfo.id == OUTPUT_STATION_ID)
    {
        return QSizeF(INPUT_OUTPUT_DIAMETER, INPUT_OUTPUT_DIAMETER);
    }

    QSizeF labelSize = getLabelSize();

    QSizeF processorTaskSize = getProcessorTaskSize();
    QSizeF queueTaskSize = getQueueTaskSize();

    int queueDrawLength = std::max(1, m_stationInfo.queueLength);

    qreal width = QUEUE_SPACING + queueDrawLength * queueTaskSize.width() + QUEUE_SPACING +
                  PROCESSOR_SPACING + processorTaskSize.width() + PROCESSOR_SPACING;

    qreal queueAreaHeight = labelSize.height() + QUEUE_SPACING + queueTaskSize.height() + QUEUE_SPACING;
    qreal processorAreaHeight = PROCESSOR_SPACING + (PROCESSOR_SPACING + processorTaskSize.height()) * m_stationInfo.processorCount;

    qreal height = std::max(queueAreaHeight, processorAreaHeight);

    return QSizeF(width, height);
}

QSizeF StationItem::getLabelSize() const
{
    QFontMetricsF metrics(m_labelFont);
    return metrics.size(0, getLabel()) + QSizeF(2*LABEL_RECT_SPACING, 2*LABEL_RECT_SPACING);
}

QSizeF StationItem::getProcessorTaskSize() const
{
    QFontMetricsF metrics(m_taskFont);
    QSizeF baseSize = metrics.size(0, "-*99");
    return baseSize + QSizeF(2*TASK_RECT_SPACING, 2*TASK_RECT_SPACING);
}

QSizeF StationItem::getQueueTaskSize() const
{
    QFontMetricsF metrics(m_taskFont);

    QSizeF baseSize;
    if (m_stationInfo.queueLength == 0)
    {
        baseSize = metrics.size(0, "#999");
    }
    else
    {
        baseSize = metrics.size(0, "-*99");
    }

    return baseSize + QSizeF(2*TASK_RECT_SPACING, 2*TASK_RECT_SPACING);
}

QString StationItem::getLabel() const
{
    QString label;
    label.setNum(m_stationInfo.id);
    return label;
}

QString StationItem::getTaskText(int number)
{
    int absNumber = std::abs(number);

    int numberOfDigits = static_cast<int>(std::ceil(std::log10(1.0 + static_cast<double>(absNumber))));

    QString text;
    if (numberOfDigits <= 2)
    {
        text.setNum(number);
    }
    else
    {
        text = QString("%1*%2").arg((number < 0) ? "-" : "").arg(absNumber % 100, 2, 10, QChar('0'));
    }

    return text;
}

void StationItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    QSizeF baseSize = getBaseSize();

    painter->translate(-baseSize.width() / 2.0, -baseSize.height() / 2.0);

    if (isSelected())
    {
        paintSelectionMarkers(painter, baseSize);
    }

    if (m_stationInfo.id == INPUT_STATION_ID ||
        m_stationInfo.id == OUTPUT_STATION_ID)
    {
        paintInputOutputNode(painter, baseSize);
        return;
    }

    QSizeF labelSize = getLabelSize();
    QSizeF processorTaskSize = getProcessorTaskSize();
    QSizeF queueTaskSize = getQueueTaskSize();

    paintBoundingRect(painter, baseSize);

    paintLabel(painter, labelSize);

    paintQueue(painter, labelSize, queueTaskSize, processorTaskSize);

    paintProcessors(painter, queueTaskSize, processorTaskSize);
}

void StationItem::paintSelectionMarkers(QPainter* painter, const QSizeF& baseSize)
{
    painter->translate(-SELECTION_MARKER_SIZE, -SELECTION_MARKER_SIZE);

    QSizeF markerSize(SELECTION_MARKER_SIZE, SELECTION_MARKER_SIZE);

    painter->setPen(QPen(Qt::black));
    painter->setBrush(QBrush(Qt::black));

    painter->drawRect(QRectF(QPointF(0.0, 0.0), markerSize));
    painter->drawRect(QRectF(QPointF(SELECTION_MARKER_SIZE + baseSize.width(), 0.0), markerSize));
    painter->drawRect(QRectF(QPointF(0.0, SELECTION_MARKER_SIZE + baseSize.height()), markerSize));
    painter->drawRect(QRectF(QPointF(SELECTION_MARKER_SIZE + baseSize.width(),
                                        SELECTION_MARKER_SIZE + baseSize.height()),
                                markerSize));

    painter->translate(SELECTION_MARKER_SIZE, SELECTION_MARKER_SIZE);
}

void StationItem::paintInputOutputNode(QPainter* painter, const QSizeF& baseSize)
{
    QRectF baseRect(QRectF(QPointF(0, 0), baseSize));

    painter->setBrush(QBrush(Qt::NoBrush));
    painter->setPen(QPen(Qt::black));

    painter->drawEllipse(baseRect);

    QString text;
    if (m_stationInfo.id == INPUT_STATION_ID)
    {
        text = "WE";
    }
    else
    {
        text = "WY";
    }

    painter->setFont(m_labelFont);
    painter->setBrush(QBrush(Qt::NoBrush));
    painter->setPen(QPen(Qt::darkBlue));
    painter->drawText(baseRect, Qt::AlignCenter, text);
}

void StationItem::paintBoundingRect(QPainter* painter, const QSizeF& baseSize)
{
    painter->setBrush(QBrush(Qt::NoBrush));
    painter->setPen(QPen(Qt::black));

    painter->drawRect(QRectF(QPointF(0.0, 0.0), baseSize));
}

void StationItem::paintLabel(QPainter* painter, const QSizeF& labelSize)
{
    painter->setBrush(QBrush(Qt::NoBrush));
    painter->setPen(QPen(Qt::darkBlue));
    painter->setFont(m_labelFont);

    QRectF labelRect(QPointF(0.0, 0.0), labelSize);
    painter->drawRect(labelRect);
    painter->drawText(labelRect, Qt::AlignCenter, getLabel());
}

void StationItem::paintQueue(QPainter* painter, const QSizeF& labelSize, const QSizeF& queueTaskSize, const QSizeF& processorTaskSize)
{
    painter->setFont(m_taskFont);

    qreal queueAreaHeight = labelSize.height() + QUEUE_SPACING + queueTaskSize.height() + QUEUE_SPACING;
    qreal processorAreaHeight = PROCESSOR_SPACING + (PROCESSOR_SPACING + processorTaskSize.height()) * m_stationInfo.processorCount;

    qreal queueHeight = 0.0;

    if (queueAreaHeight > processorAreaHeight)
    {
        queueHeight = labelSize.height() + QUEUE_SPACING;
    }
    else
    {
        queueHeight = std::max(processorAreaHeight / 2.0 - queueTaskSize.height() / 2.0,
                               labelSize.height() + QUEUE_SPACING);
    }

    QPointF queuePos(QUEUE_SPACING, queueHeight);

    painter->setPen(QPen(Qt::black));

    int queueDrawLength = std::max(1, m_stationInfo.queueLength);

    for (int i = 0; i < queueDrawLength; ++i)
    {
        QString taskText;
        if (m_stationInfo.queueLength == 0)
        {
            taskText = QString("#%1").arg(m_tasksInQueue.size());
        }
        else
        {
            int task = m_tasksInQueue.at(m_tasksInQueue.size() - i - 1);
            if (task != EMPTY_TASK_ID)
            {
                taskText = getTaskText(task);
            }
        }

        QRectF queueTaskRect(queuePos, queueTaskSize);
        painter->drawRect(queueTaskRect);
        painter->drawText(queueTaskRect, Qt::AlignCenter, taskText);

        queuePos += QPointF(queueTaskSize.width(), 0.0);
    }
}

void StationItem::paintProcessors(QPainter* painter, const QSizeF& queueTaskSize, const QSizeF& processorTaskSize)
{
    qreal queueAreaWidth = QUEUE_SPACING + std::max(1, m_stationInfo.queueLength) * queueTaskSize.width() + QUEUE_SPACING;

    QPointF processorPos(queueAreaWidth + PROCESSOR_SPACING, PROCESSOR_SPACING);

    for (int i = 0; i < m_stationInfo.processorCount; ++i)
    {
        QString taskText;
        int task = m_tasksInProcessors.at(i);
        if (task != EMPTY_TASK_ID)
        {
            taskText = getTaskText(m_tasksInProcessors.at(i));
        }

        QRectF processorTaskRect(processorPos, processorTaskSize);
        painter->drawRect(processorTaskRect);
        painter->drawText(processorTaskRect, Qt::AlignCenter, taskText);

        processorPos += QPointF(0.0, processorTaskSize.height() + PROCESSOR_SPACING);
    }
}
