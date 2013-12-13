#include "ui/simulation_scene.hpp"

#include "engine/simulation_instance.hpp"

#include "ui/connection_item.hpp"
#include "ui/station_item.hpp"


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
    addItem(connectionItem);

    m_connectionItems.append(connectionItem);
}

int SimulationScene::getSelectedStationId() const
{
    QList<QGraphicsItem*> selectedItems = QGraphicsScene::selectedItems();
    if (selectedItems.size() != 1)
    {
        return -1;
    }

    QGraphicsItem* selectedItem = selectedItems.at(0);

    for (StationItem* stationItem : m_stationItems)
    {
        if (stationItem == selectedItem)
        {
            return stationItem->getId();
        }
    }

    return -1;
}

QPair<int, int> SimulationScene::getSelectedConnection() const
{
    QList<QGraphicsItem*> selectedItems = QGraphicsScene::selectedItems();
    if (selectedItems.size() != 1)
    {
        return qMakePair(-1, -1);
    }

    QGraphicsItem* selectedItem = selectedItems.at(0);

    for (ConnectionItem* connectionItem : m_connectionItems)
    {
        if (connectionItem == selectedItem)
        {
            if (connectionItem->getSource() != nullptr && connectionItem->getDestination() != nullptr)
            {
                int sourceId = connectionItem->getSource()->getId();
                int destinationId = connectionItem->getDestination()->getId();
                return qMakePair<int, int>(sourceId, destinationId);
            }
        }
    }

    return qMakePair<int, int>(-1, -1);
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

void SimulationScene::processMousePress(const QPointF& scenePos)
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

void SimulationScene::processMouseMove(const QPointF& scenePos)
{
    if (m_newConnection != nullptr)
    {
        m_newConnection->setMovePoint(scenePos);
    }
}

void SimulationScene::processMouseRelease(const QPointF& scenePos)
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

ConnectionItem* SimulationScene::getConnectionItemByIds(int from, int to)
{
    for (ConnectionItem* connectionItem : m_connectionItems)
    {
        if (connectionItem->getSource() != nullptr && connectionItem->getDestination() != nullptr)
        {
            if (connectionItem->getSource()->getId() == from && connectionItem->getDestination()->getId() == to)
            {
                return connectionItem;
            }
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

