#include "ui/simulation_scene.hpp"

#include "engine/simulation_instance.hpp"

#include "ui/connection_item.hpp"
#include "ui/station_item.hpp"


SimulationScene::SimulationScene(QObject* parent)
 : QGraphicsScene(parent)
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

void SimulationScene::addStation(const Station& stationInfo)
{
    StationItem* stationItem = new StationItem();
    addItem(stationItem);
    stationItem->updateInfo(stationInfo);
    stationItem->setPos(stationInfo.position);
    m_stationItems[stationInfo.id] = stationItem;
}

void SimulationScene::addConnection(const Connection& connectionInfo)
{
    StationItem* source = nullptr;
    if (m_stationItems.contains(connectionInfo.from))
    {
        source = m_stationItems[connectionInfo.from];
    }

    StationItem* destination = nullptr;
    if (m_stationItems.contains(connectionInfo.to))
    {
        destination = m_stationItems[connectionInfo.to];
    }

    ConnectionItem* connectionItem = new ConnectionItem(source, destination);
    addItem(connectionItem);

    m_connectionItems[qMakePair<int,int>(connectionInfo.from, connectionInfo.to)] = connectionItem;
}

int SimulationScene::getSelectedStationId() const
{
    QList<QGraphicsItem*> selectedItems = QGraphicsScene::selectedItems();
    if (selectedItems.size() != 1)
    {
        return -1;
    }

    QGraphicsItem* selectedItem = selectedItems.at(0);

    for (auto it = m_stationItems.begin(); it != m_stationItems.end(); ++it)
    {
        if (it.value() == selectedItem)
        {
            return it.key();
        }
    }

    return -1;
}

void SimulationScene::changeStation(int id, const Station& stationParams)
{
    if (m_stationItems.contains(id))
    {
        StationItem* stationItem = m_stationItems[id];
        stationItem->updateInfo(stationParams);
    }
}

