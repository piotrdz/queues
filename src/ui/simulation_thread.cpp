#include "ui/simulation_thread.hpp"

#include "engine/simulation.hpp"

#include <QElapsedTimer>


SimulationThread::SimulationThread(QObject* parent, Simulation* simulation)
 : QThread(parent)
 , m_simulation(simulation)
 , m_speed(1.0)
 , m_state(State::Idle)
 , m_speedChanged(false)
 , m_end(false)
{
}

void SimulationThread::startSimulation()
{
    m_mutex.lock();
    m_state = State::Running;
    m_waitCondition.wakeOne();
    m_mutex.unlock();
}

void SimulationThread::stopSimulation()
{
    m_mutex.lock();
    m_state = State::Idle;
    m_waitCondition.wakeOne();
    m_mutex.unlock();
}

void SimulationThread::resetSimulation()
{
    m_mutex.lock();
    m_state = State::Idle;
    m_simulation->reset();
    m_waitCondition.wakeOne();
    m_mutex.unlock();
}

void SimulationThread::setSimulationSpeed(double speed)
{
    m_mutex.lock();
    m_speed = speed;
    m_speedChanged = true;
    m_waitCondition.wakeOne();
    m_mutex.unlock();
}

void SimulationThread::singleStepSimulation()
{
    m_mutex.lock();
    m_state = State::SingleStep;
    m_waitCondition.wakeOne();
    m_mutex.unlock();
}

void SimulationThread::endThread()
{
    m_mutex.lock();
    m_end = true;
    m_waitCondition.wakeOne();
    m_mutex.unlock();
}

SimulationThread::SimulationInfo SimulationThread::getSimulationInfo()
{
    SimulationInfo info;
    m_mutex.lock();
    info.state = m_state;
    info.lastEventTime = m_simulation->getCurrentTime();
    info.currentSimulationTime = m_simulation->getCurrentTime();
    if (m_state == SimulationThread::State::Running)
    {
        info.currentSimulationTime += (m_elapsedTimer.elapsed() / 1000.0) * m_speed;
    }
    info.nextEventTime = m_simulation->getCurrentTime() + m_simulation->getTimeToNextStep();
    m_mutex.unlock();
    return info;
}

SimulationThread::State SimulationThread::getState()
{
    State state;
    m_mutex.lock();
    state = m_state;
    m_mutex.unlock();
    return state;
}

void SimulationThread::run()
{
    m_mutex.lock();

    m_elapsedTimer.start();

    while (!m_end)
    {
        unsigned long waitTime = ULONG_MAX;

        if (!m_speedChanged && (m_state == State::Running || m_state == State::SingleStep))
        {
            Event event = m_simulation->simulateNextStep();
            m_simulation->debugDump();
            emit newEvent(event);
            m_elapsedTimer.start();
        }

        if (m_speedChanged || m_state == State::Running)
        {
            double time = m_simulation->getTimeToNextStep();
            waitTime = static_cast<unsigned long>(time * 1000 / m_speed);
            m_speedChanged = false;
        }

        if (m_state == State::SingleStep)
        {
            m_state = State::Idle;
        }

        m_waitCondition.wait(&m_mutex, waitTime);
    }

    m_mutex.unlock();
}
