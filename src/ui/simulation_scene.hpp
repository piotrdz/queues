#pragma once

#include <QGraphicsScene>
#include <QMap>

class Connection;
class ConnectionItem;
class Station;
class StationItem;
class SimulationInstance;

class SimulationScene : public QGraphicsScene
{
public:
    explicit SimulationScene(QObject* parent);
    virtual ~SimulationScene();

    void clear();

    void setSimulationInstance(const SimulationInstance& simulationInstance);

    void addStation(const Station& station);
    void addConnection(const Connection& connection);

private:
    QMap<int, StationItem*> m_stationItems;
    QMap<QPair<int,int>, ConnectionItem*> m_connectionItems;
};
