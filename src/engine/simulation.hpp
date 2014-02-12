#pragma once

#include "engine/event.hpp"
#include "engine/event_priority_queue.hpp"
#include "engine/simulation_instance.hpp"

#include <QList>
#include <QQueue>

#include <boost/random.hpp>

class Simulation
{
private:
    struct WorkingStation : public Station
    {
        WorkingStation(const Station& station);

        void resetStateParams();

        QList<int> tasksInQueue;
        QList<int> tasksInProcessors;
    };

    struct WorkingInstance
    {
        WorkingInstance();
        WorkingInstance(const SimulationInstance& simulationInstance);
        WorkingInstance& operator=(const SimulationInstance& simulationInstance);

        void setStations(const QList<Station>& stations);

        SimulationInstance toSimulationInstance() const;

        Distribution arrivalTimeDistribution;
        QList<WorkingStation> workingStations;
        QList<Connection> connections;
    };

public:
    Simulation();

    void setInstance(const SimulationInstance& instance);
    SimulationInstance getInstance() const;

    void addStation(const Station& station);
    void addConnection(const Connection& connection);

    void changeArrivalDistribution(const Distribution& distribution);

    int getNextStationId() const;
    Station getStation(int id) const;
    void changeStation(int id, const StationParams& stationParams);

    int getConnectionWeight(int from, int to) const;
    void changeConnectionWeight(int from, int to, int weight);

    bool connectionExists(int from, int to) const;
    bool isConnectionPossible(int from, int to) const;

    void removeStation(int id);
    void removeConnection(int from, int to);

    void updateStationPositions(const QMap<int, QPointF>& positions);

    void reset();
    Event simulateNextStep();
    double getCurrentTime();
    double getTimeToNextStep();

    bool check() const;
    static bool check(const SimulationInstance& instance);

    static SimulationInstance readFromFile(const QString& path);
    static void saveToFile(const QString& path, const SimulationInstance& simulationInstance);

    void debugDump();

private:
    void processEvent(Event event);
    void processTaskInput(Event event);
    void processTaskAddedToQueue(Event event);
    void processTaskStartedProcessing(Event event);
    void processTaskEndedProcessing(Event event);
    void processTaskQueueHasPlace(Event event);
    void processTaskMachineIsIdle(Event event);

    int generateTaskId();
    QList<Connection> getConnectionsFrom(int stationId) const;
    QList<Connection> getConnectionsTo(int stationId) const;
    WorkingStation& getWorkingStation(int stationId);

    double generateTime(const Distribution& distribution);
    Connection chooseConnectionToFollow(const QList<Connection>& connections);
    int chooseRandomTaskFromQueue(const QList<int>& tasks);

private:
    WorkingInstance m_instance;
    EventPriorityQueue m_eventQueue;
    int m_nextStationId;
    int m_nextTaskId;
    double m_currentTime;
    boost::random::mt19937 m_randomGenerator;
};
