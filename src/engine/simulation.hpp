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

    void reset();
    Event simulateNextStep();
    double getCurrentTime();
    double getTimeToNextStep();

private:
    double m_currentTime;
    SimulationInstance m_instance;
};
