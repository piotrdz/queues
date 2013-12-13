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

    int getSelectedStationId() const;
    QPair<int, int> getSelectedConnection() const;

    void changeStation(int id, const Station& stationParams);
    void changeConnectionWeight(int from, int to, int weight);

private:
    QMap<int, StationItem*> m_stationItems;
    QMap<QPair<int, int>, ConnectionItem*> m_connectionItems;
};
