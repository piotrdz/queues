#pragma once

#include "engine/event.hpp"

#include <QElapsedTimer>
#include <QMutex>
#include <QThread>
#include <QWaitCondition>

class Simulation;

class SimulationThread : public QThread
{
    Q_OBJECT
public:
    enum class State
    {
        Idle,
        Running,
        SingleStep
    };

    struct SimulationInfo
    {
        State state;
        double lastEventTime;
        double currentSimulationTime;
        double nextEventTime;
    };

public:
    explicit SimulationThread(QObject* parent, Simulation* simulation);

    void stopSimulation();
    void startSimulation();
    void resetSimulation();
    void endThread();

    SimulationInfo getSimulationInfo();
    State getState();

public slots:
    void setSimulationSpeed(double speed);
    void singleStepSimulation();

protected:
    virtual void run() override;

signals:
    void newEvent(Event event);

private:
    Simulation* m_simulation;
    double m_speed;
    State m_state;
    bool m_speedChanged;
    bool m_end;
    QMutex m_mutex;
    QWaitCondition m_waitCondition;
    QElapsedTimer m_elapsedTimer;
};
