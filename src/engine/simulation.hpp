#pragma once

#include "engine/event.hpp"
#include "engine/simulation_instance.hpp"

#include <QList>

class Simulation
{
public:
    Simulation();

    void setInstance(const SimulationInstance& instance);

    void addStation(const Station& station);
    void addConnection(const Connection& connection);
    bool check();

    void changeArrivalDistribution(const Distribution& distribution);

    int getNextStationId() const;
    Station getStation(int id) const;
    void changeStation(int id, const StationParams& stationParams);

    int getConnectionWeight(int from, int to) const;
    void changeConnectionWeight(int from, int to, int weight);

    bool connectionExists(int from, int to) const;
    bool isConnectionPossible(int from, int to) const;

    void reset();
    Event simulateNextStep();
    double getCurrentTime();
    double getTimeToNextStep();

private:
    double m_currentTime;
    SimulationInstance m_instance;
    int m_nextStationId;
};
