#pragma once

#include <QGraphicsScene>
#include <QList>

class Connection;
class ConnectionItem;
class Station;
class StationParams;
class StationItem;
class SimulationInstance;

class SimulationScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit SimulationScene(QObject* parent);
    virtual ~SimulationScene();

    void clear();

    void setSimulationInstance(const SimulationInstance& simulationInstance);

    void addStation(const Station& station);
    void addConnection(const Connection& connection);

    int getSelectedStationId() const;
    QPair<int, int> getSelectedConnection() const;

    void changeStation(int id, const StationParams& stationParams);
    void changeConnectionWeight(int from, int to, int weight);

    void removeStation(int id);
    void removeConnection(int from, int to);

    void processCustomMousePress(const QPointF& scenePos);
    void processCustomMouseMove(const QPointF& scenePos);
    void processCustomMouseRelease(const QPointF& scenePos);

signals:
    void stationAddRequest(const QPointF& pos);
    void connectionAddRequest(int from, int to);

    void stationRemoveRequest(int id);
    void connectionRemoveRequest(int from, int to);

protected:
    virtual void keyPressEvent(QKeyEvent* event) override;

private:
    StationItem* getStationItemById(int id);
    StationItem* getStationItemAtPos(const QPointF& pos);
    ConnectionItem* getConnectionItemByIds(int from, int to);

private:
    QList<StationItem*> m_stationItems;
    QList<ConnectionItem*> m_connectionItems;
    ConnectionItem* m_newConnection;
    int m_nextStationId;
    QObject* m_viewEventFilter;
};
