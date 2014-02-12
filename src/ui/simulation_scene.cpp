#include "ui/simulation_scene.hpp"

#include "engine/simulation_instance.hpp"

#include "ui/connection_item.hpp"
#include "ui/station_item.hpp"

#include <QKeyEvent>


SimulationScene::SimulationScene(QObject* parent)
 : QGraphicsScene(parent)
 , m_newConnection(nullptr)
{
}

SimulationScene::~SimulationScene()
{
}

void SimulationScene::clear()
{
    QList<QGraphicsItem*> items = QGraphicsScene::items();
    for (QGraphicsItem* item : items)
    {
        removeItem(item);
        delete item;
    }

    m_connectionItems.clear();
    m_stationItems.clear();
}

void SimulationScene::setSimulationInstance(const SimulationInstance& simulationInstance)
{
    clear();

    for (const Station& station : simulationInstance.stations)
    {
        addStation(station);
    }

    for (const Connection& connection : simulationInstance.connections)
    {
        addConnection(connection);
    }
}

void SimulationScene::addStation(const Station& station)
{
    Q_ASSERT(getStationItemById(station.id) == nullptr);

    StationItem* stationItem = new StationItem(station);
    addItem(stationItem);
    stationItem->setPos(station.position);
    m_stationItems.append(stationItem);
}

void SimulationScene::addConnection(const Connection& connection)
{
    Q_ASSERT(getConnectionItemByIds(connection.from, connection.to) == nullptr);

    StationItem* source = getStationItemById(connection.from);
    Q_ASSERT(source != nullptr);

    StationItem* destination = getStationItemById(connection.to);
    Q_ASSERT(destination != nullptr);

    ConnectionItem* connectionItem = new ConnectionItem(source, destination);
    connectionItem->updateWeight(connection.weight);
    addItem(connectionItem);

    m_connectionItems.append(connectionItem);
}

int SimulationScene::getSelectedStationId() const
{
    QList<QGraphicsItem*> selectedItems = QGraphicsScene::selectedItems();
    if (selectedItems.size() != 1)
    {
        return INVALID_STATION_ID;
    }

    QGraphicsItem* selectedItem = selectedItems.at(0);

    for (StationItem* stationItem : m_stationItems)
    {
        if (stationItem == selectedItem)
        {
            return stationItem->getId();
        }
    }

    return INVALID_STATION_ID;
}

QPair<int, int> SimulationScene::getSelectedConnection() const
{
    QList<QGraphicsItem*> selectedItems = QGraphicsScene::selectedItems();
    if (selectedItems.size() != 1)
    {
        return qMakePair(INVALID_STATION_ID, INVALID_STATION_ID);
    }

    QGraphicsItem* selectedItem = selectedItems.at(0);

    for (ConnectionItem* connectionItem : m_connectionItems)
    {
        if (connectionItem == selectedItem)
        {
            int sourceId = connectionItem->getSource()->getId();
            int destinationId = connectionItem->getDestination()->getId();
            return qMakePair<int, int>(sourceId, destinationId);
        }
    }

    return qMakePair<int, int>(INVALID_STATION_ID, INVALID_STATION_ID);
}

void SimulationScene::changeStation(int id, const StationParams& stationParams)
{
    StationItem* stationItem = getStationItemById(id);

    if (stationItem != nullptr)
    {
        stationItem->updateParams(stationParams);
    }
}

void SimulationScene::changeConnectionWeight(int from, int to, int weight)
{
    ConnectionItem* connectionItem = getConnectionItemByIds(from, to);

    if (connectionItem != nullptr)
    {
        connectionItem->updateWeight(weight);
    }
}

void SimulationScene::removeStation(int id)
{
    for (auto stationIt = m_stationItems.begin(); stationIt != m_stationItems.end(); ++stationIt)
    {
        StationItem* stationItem = *stationIt;
        if (stationItem->getId() == id)
        {
            removeItem(stationItem);
            delete stationItem;
            m_stationItems.erase(stationIt);
            break;
        }
    }

    auto connectionIt = m_connectionItems.begin();
    while (connectionIt != m_connectionItems.end())
    {
        ConnectionItem* connectionItem = *connectionIt;
        if (connectionItem->getSource() == nullptr || connectionItem->getDestination() == nullptr)
        {
            removeItem(connectionItem);
            delete connectionItem;
            connectionIt = m_connectionItems.erase(connectionIt);
        }
        else
        {
            ++connectionIt;
        }
    }
}

void SimulationScene::removeConnection(int from, int to)
{
    for (auto connectionIt = m_connectionItems.begin(); connectionIt != m_connectionItems.end(); ++connectionIt)
    {
        ConnectionItem* connectionItem = *connectionIt;
        if (connectionItem->getSource()->getId() == from && connectionItem->getDestination()->getId() == to)
        {
            removeItem(connectionItem);
            delete connectionItem;
            m_connectionItems.erase(connectionIt);
            break;
        }
    }
}

QMap<int, QPointF> SimulationScene::getStationPositions() const
{
    QMap<int, QPointF> positions;

    for (StationItem* stationItem : m_stationItems)
    {
        positions[stationItem->getId()] = stationItem->pos();
    }

    return positions;
}

void SimulationScene::processCustomMousePress(const QPointF& scenePos)
{
    StationItem* stationItem = getStationItemAtPos(scenePos);
    if (stationItem != nullptr)
    {
        m_newConnection = new ConnectionItem(stationItem, nullptr, true);
        m_newConnection->setMovePoint(scenePos);
        addItem(m_newConnection);
    }
    else
    {
        emit stationAddRequest(scenePos);
    }
}

void SimulationScene::processCustomMouseMove(const QPointF& scenePos)
{
    if (m_newConnection != nullptr)
    {
        m_newConnection->setMovePoint(scenePos);
    }
}

void SimulationScene::processCustomMouseRelease(const QPointF& scenePos)
{
    if (m_newConnection != nullptr)
    {
        StationItem* stationItem = getStationItemAtPos(scenePos);
        if (stationItem != nullptr)
        {
            emit connectionAddRequest(m_newConnection->getSource()->getId(), stationItem->getId());
        }

        removeItem(m_newConnection);
        delete m_newConnection;
        m_newConnection = nullptr;
    }
}

void SimulationScene::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Delete)
    {
        int stationId = getSelectedStationId();
        if (stationId != INVALID_STATION_ID)
        {
            emit stationRemoveRequest(stationId);
        }
        else
        {
            QPair<int, int> connection = getSelectedConnection();
            if (connection.first != INVALID_STATION_ID)
            {
                emit connectionRemoveRequest(connection.first, connection.second);
            }
        }

        event->ignore();
        return;
    }

    QGraphicsScene::keyPressEvent(event);
}

ConnectionItem* SimulationScene::getConnectionItemByIds(int from, int to)
{
    for (ConnectionItem* connectionItem : m_connectionItems)
    {
        if (connectionItem->getSource()->getId() == from && connectionItem->getDestination()->getId() == to)
        {
            return connectionItem;
        }
    }

    return nullptr;
}

StationItem* SimulationScene::getStationItemById(int id)
{
    for (StationItem* stationItem : m_stationItems)
    {
        if (stationItem->getId() == id)
        {
            return stationItem;
        }
    }

    return nullptr;
}

StationItem* SimulationScene::getStationItemAtPos(const QPointF& pos)
{
    QList<QGraphicsItem*> items = QGraphicsScene::items(pos);
    if (items.size() < 1)
    {
        return nullptr;
    }

    for (QGraphicsItem* item : items)
    {
        for (StationItem* stationItem : m_stationItems)
        {
            if (stationItem == item)
            {
                return stationItem;
            }
        }
    }

    return nullptr;
}

void SimulationScene::newEvent(Event event)
{
    if (event.type == EventType::TaskAddedToQueue ||
        event.type == EventType::TaskStartedProcessing ||
        event.type == EventType::TaskEndedProcessing ||
        event.type == EventType::MachineIsIdle)
    {
        StationItem* stationItem = getStationItemById(event.stationId);
        Q_ASSERT(stationItem != nullptr);

        stationItem->newEvent(event);
        update();
    }
}

void SimulationScene::reset()
{
    for (StationItem* stationItem : m_stationItems)
    {
        stationItem->reset();
    }
}

