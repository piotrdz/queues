#include "engine/simulation.hpp"


Simulation::Simulation()
 : m_currentTime(0.0)
{}

void Simulation::setInstance(const SimulationInstance& instance)
{
    // TODO
}

void Simulation::addStation(const Station& station)
{
    // TODO
}

void Simulation::addConnection(const Connection& connection)
{
    // TODO
}

bool Simulation::check()
{
    // TODO
    return true;
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
