#pragma once

#include "engine/event.hpp"
#include "engine/connection.hpp"
#include "engine/station.hpp"

#include <QList>

class Simulation
{
public:
    Simulation();

    void addStation(const Station& station);
    void addConnection(const Connection& connection);
    bool check();

    void reset();
    Event simulateNextStep();
    double getCurrentTime();
    double getTimeToNextStep();

private:
    double m_currentTime;
    QList<Station> m_stations;
    QList<Connection> m_connections;
};
