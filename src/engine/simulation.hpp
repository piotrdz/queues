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

    Station getStation(int id) const;
    void changeStation(int id, const Station& stationParams);

    int getConnectionWeight(int from, int to) const;
    void changeConnectionWeight(int from, int to, int weight);

    void reset();
    Event simulateNextStep();
    double getCurrentTime();
    double getTimeToNextStep();

private:
    double m_currentTime;
    SimulationInstance m_instance;
};
