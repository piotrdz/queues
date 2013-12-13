#pragma once

#include "engine/event.hpp"

#include <QMainWindow>

namespace Ui
{
    class MainWindow;
}

class Simulation;
class SimulationScene;
class SimulationThread;
class SimulationInstance;
class StatisticsWindow;

class QLabel;
class QTimer;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    virtual ~MainWindow();

    void setSimulationInstance(const SimulationInstance& simulationInstance);
    void setStatisticsWindow(StatisticsWindow* statisticsWindow);

protected:
    void closeEvent(QCloseEvent* e);

private slots:
    void arrivalDistributionParamsChanged();

    void updateStationParams();
    void stationParamsChanged();

    void resetClicked();
    void startStopClicked();
    void newEvent(Event event);
    void updateSimulationInfo();

private:
    void connectStationParamsWidgets();
    void disconnectStationParamsWidgets();

    void setSampleSimulationInstance();

private:
    Ui::MainWindow* m_ui;
    Simulation* m_simulation;
    SimulationScene* m_simulationScene;
    SimulationThread* m_simulationThread;
    StatisticsWindow* m_statisticsWindow;
    QTimer* m_updateInfoTimer;
    QLabel* m_simulationStateLabel;
    QLabel* m_lastEventTimeLabel;
    QLabel* m_simulationTimeLabel;
    QLabel* m_nextEventTimeLabel;
};

