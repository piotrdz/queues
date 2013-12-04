#include "ui/main_window.hpp"

#include "engine/simulation.hpp"

#include "ui/connection_item.hpp"
#include "ui/simulation_scene.hpp"
#include "ui/simulation_thread.hpp"
#include "ui/station_item.hpp"
#include "ui/statistics_window.hpp"

#include "ui_main_window.h"

#include <QDebug>
#include <QTimer>


MainWindow::MainWindow()
 : m_ui(new Ui::MainWindow())
 , m_simulation(nullptr)
 , m_simulationScene(nullptr)
 , m_simulationThread(nullptr)
 , m_statisticsWindow(nullptr)
 , m_updateInfoTimer(nullptr)
 , m_simulationStateLabel(nullptr)
 , m_lastEventTimeLabel(nullptr)
 , m_simulationTimeLabel(nullptr)
 , m_nextEventTimeLabel(nullptr)
{
    m_ui->setupUi(this);

    m_simulation = new Simulation();

    m_simulationThread = new SimulationThread(this, m_simulation);
    m_simulationThread->start();

    m_simulationScene = new SimulationScene(this);
    m_ui->simulationView->setScene(m_simulationScene);

    m_updateInfoTimer = new QTimer(this);
    m_updateInfoTimer->start(40);

    m_simulationStateLabel = new QLabel(this);
    m_ui->statusBar->addWidget(m_simulationStateLabel, 1);

    m_lastEventTimeLabel = new QLabel(this);
    m_ui->statusBar->addWidget(m_lastEventTimeLabel, 1);

    m_simulationTimeLabel = new QLabel(this);
    m_ui->statusBar->addWidget(m_simulationTimeLabel, 1);

    m_nextEventTimeLabel = new QLabel(this);
    m_ui->statusBar->addWidget(m_nextEventTimeLabel, 1);

    Station station1Info;
    station1Info.id = 1;
    station1Info.processorCount = 1;
    station1Info.queueLength = 2;

    StationItem* station1 = new StationItem();
    m_simulationScene->addItem(station1);
    station1->updateInfo(station1Info);
    station1->setPos(-50, -50);

    Station station2Info;
    station2Info.id = 2;
    station2Info.processorCount = 2;
    station2Info.queueLength = 3;

    StationItem* station2 = new StationItem();
    m_simulationScene->addItem(station2);
    station2->updateInfo(station2Info);
    station2->setPos(50, 50);

    m_simulationScene->addItem(new ConnectionItem(station1, station2));

    connect(m_simulationThread, SIGNAL(newEvent(Event)),
            this, SLOT(newEvent(Event)));

    connect(m_ui->resetButton, SIGNAL(clicked()),
            this, SLOT(resetClicked()));

    connect(m_ui->startStopButton, SIGNAL(clicked()),
            this, SLOT(startStopClicked()));

    connect(m_ui->singleStepButton, SIGNAL(clicked()),
            m_simulationThread, SLOT(singleStepSimulation()));

    connect(m_ui->speedSpinBox, SIGNAL(valueChanged(double)),
            m_simulationThread, SLOT(setSimulationSpeed(double)));

    connect(m_updateInfoTimer, SIGNAL(timeout()),
            this, SLOT(updateSimulationInfo()));
}

MainWindow::~MainWindow()
{
    delete m_ui;
    m_ui = nullptr;

    delete m_simulation;
    m_simulation = nullptr;

    delete m_simulationScene;
    m_simulationScene = nullptr;

    delete m_simulationThread;
    m_simulationThread = nullptr;

    delete m_updateInfoTimer;
    m_updateInfoTimer = nullptr;
}

void MainWindow::closeEvent(QCloseEvent* e)
{
    m_simulationThread->endThread();

    QWidget::closeEvent(e);
}

void MainWindow::setStatisticsWindow(StatisticsWindow* statisticsWindow)
{
    m_statisticsWindow = statisticsWindow;
    connect(m_simulationThread, SIGNAL(newEvent(Event)),
            m_statisticsWindow, SLOT(newEvent(Event)));
}

void MainWindow::resetClicked()
{
    m_ui->singleStepButton->setEnabled(true);
    m_ui->startStopButton->setText("Start");
    m_simulationThread->resetSimulation();
}

void MainWindow::startStopClicked()
{
    if (m_simulationThread->getState() == SimulationThread::State::Running)
    {
        m_ui->singleStepButton->setEnabled(true);
        m_ui->startStopButton->setText("Start");
        m_simulationThread->stopSimulation();
    }
    else
    {
        m_ui->singleStepButton->setEnabled(false);
        m_ui->startStopButton->setText("Stop");
        m_simulationThread->startSimulation();
    }
}

void MainWindow::updateSimulationInfo()
{
    SimulationThread::SimulationInfo info = m_simulationThread->getSimulationInfo();

    QString simulationState;
    if (info.state == SimulationThread::State::Running)
    {
        simulationState = "uruchomiona";
    }
    else
    {
        simulationState = "wstrzymana";
    }

    m_simulationStateLabel->setText(QString("Symulacja: %1").arg(simulationState));
    m_lastEventTimeLabel->setText(QString("Czas ost. zdarz.: %1 s").arg(info.lastEventTime, 0, 'f', 3));
    m_simulationTimeLabel->setText(QString("Czas sym.: %1 s").arg(info.currentSimulationTime, 0, 'f', 3));
    m_nextEventTimeLabel->setText(QString("Czas nast. zdarz.: %1 s").arg(info.nextEventTime, 0, 'f', 3));
}

void MainWindow::newEvent(Event event)
{
    qDebug() << "New event!";
}

