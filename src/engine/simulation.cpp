#include "engine/simulation.hpp"


Simulation::Simulation()
 : m_currentTime(0.0)
{}

void Simulation::setInstance(const SimulationInstance& instance)
{
    m_instance = instance;
}

void Simulation::addStation(const Station& station)
{
    m_instance.stations.append(station);
}

void Simulation::addConnection(const Connection& connection)
{
    m_instance.connections.append(connection);
}

bool Simulation::check()
{
    // TODO
    return true;
}

void Simulation::changeArrivalDistribution(const Distribution& distribution)
{
    m_instance.arrivalTimeDistribution = distribution;
}

Station Simulation::getStation(int id) const
{
    for (const Station& station : m_instance.stations)
    {
        if (station.id == id)
        {
            return station;
        }
    }

    return Station();
}

void Simulation::changeStation(int id, const Station& stationParams)
{
    for (Station& station : m_instance.stations)
    {
        if (station.id == id)
        {
            station = stationParams;
            station.id = id;
        }
    }
}

void Simulation::reset()
{
    m_currentTime = 0.0;
    // TODO
}

Event Simulation::simulateNextStep()
{
    m_currentTime += getTimeToNextStep();
    // TODO
    return Event();
}

double Simulation::getCurrentTime()
{
    return m_currentTime;
}

double Simulation::getTimeToNextStep()
{
    // TODO
    return 0.5;
}
