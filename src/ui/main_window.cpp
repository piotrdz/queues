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

    setSampleSimulationInstance();

    updateStationParams();
    updateConnectionParams();


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

    connect(m_ui->arrivalDistributionParamsWidget, SIGNAL(distributionParamsChanged()),
            this, SLOT(arrivalDistributionParamsChanged()));

    connect(m_simulationScene, SIGNAL(selectionChanged()),
            this, SLOT(updateStationParams()));

    connect(m_simulationScene, SIGNAL(selectionChanged()),
            this, SLOT(updateConnectionParams()));

    connectStationParamsWidgets();
    connectConnectionParamsWidgets();
}

MainWindow::~MainWindow()
{
    m_statisticsWindow = nullptr;

    delete m_simulationScene;
    m_simulationScene = nullptr;

    delete m_simulationStateLabel;
    m_simulationStateLabel = nullptr;

    delete m_lastEventTimeLabel;
    m_lastEventTimeLabel = nullptr;

    delete m_simulationTimeLabel;
    m_simulationTimeLabel = nullptr;

    delete m_nextEventTimeLabel;
    m_nextEventTimeLabel = nullptr;

    delete m_simulation;
    m_simulation = nullptr;

    delete m_simulationThread;
    m_simulationThread = nullptr;

    delete m_updateInfoTimer;
    m_updateInfoTimer = nullptr;

    delete m_ui;
    m_ui = nullptr;
}

void MainWindow::closeEvent(QCloseEvent* e)
{
    m_simulationThread->endThread();

    QWidget::closeEvent(e);
}

void MainWindow::connectStationParamsWidgets()
{
    connect(m_ui->queueTypeComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(stationParamsChanged()));

    connect(m_ui->infiniteQueueRadioButton, SIGNAL(toggled(bool)),
            this, SLOT(stationParamsChanged()));

    connect(m_ui->finiteQueueRadioButton, SIGNAL(toggled(bool)),
            this, SLOT(stationParamsChanged()));

    connect(m_ui->queueLengthSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(stationParamsChanged()));

    connect(m_ui->processorCountSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(stationParamsChanged()));

    connect(m_ui->processingDistributionParamsWidget, SIGNAL(distributionParamsChanged()),
            this, SLOT(stationParamsChanged()));
}

void MainWindow::disconnectStationParamsWidgets()
{
    disconnect(m_ui->queueTypeComboBox, SIGNAL(currentIndexChanged(int)),
               this, SLOT(stationParamsChanged()));

    disconnect(m_ui->infiniteQueueRadioButton, SIGNAL(toggled(bool)),
               this, SLOT(stationParamsChanged()));

    disconnect(m_ui->finiteQueueRadioButton, SIGNAL(toggled(bool)),
               this, SLOT(stationParamsChanged()));

    disconnect(m_ui->queueLengthSpinBox, SIGNAL(valueChanged(int)),
               this, SLOT(stationParamsChanged()));

    disconnect(m_ui->processorCountSpinBox, SIGNAL(valueChanged(int)),
               this, SLOT(stationParamsChanged()));

    disconnect(m_ui->processingDistributionParamsWidget, SIGNAL(distributionParamsChanged()),
               this, SLOT(stationParamsChanged()));
}

void MainWindow::connectConnectionParamsWidgets()
{
    connect(m_ui->connectionWeightSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(connectionParamsChanged()));
}

void MainWindow::disconnectConnectionParamsWidgets()
{
    disconnect(m_ui->connectionWeightSpinBox, SIGNAL(valueChanged(int)),
               this, SLOT(connectionParamsChanged()));
}

void MainWindow::setSampleSimulationInstance()
{
    SimulationInstance instance;

    Station station1Info;
    station1Info.id = 1;
    station1Info.processorCount = 1;
    station1Info.queueLength = 2;
    station1Info.position = QPointF(-80, -80);
    instance.stations.append(station1Info);

    Station station2Info;
    station2Info.id = 2;
    station2Info.processorCount = 2;
    station2Info.queueLength = 3;
    station2Info.position = QPointF(80, 80);
    instance.stations.append(station2Info);

    Connection connectionInfo;
    connectionInfo.from = 1;
    connectionInfo.to = 2;
    connectionInfo.weight = 1;
    instance.connections.append(connectionInfo);

    setSimulationInstance(instance);
}

void MainWindow::setSimulationInstance(const SimulationInstance& simulationInstance)
{
    m_simulation->setInstance(simulationInstance);
    m_simulationScene->setSimulationInstance(simulationInstance);

    m_ui->arrivalDistributionParamsWidget->setDistributionParams(simulationInstance.arrivalTimeDistribution);
    updateStationParams();
}

void MainWindow::setStatisticsWindow(StatisticsWindow* statisticsWindow)
{
    m_statisticsWindow = statisticsWindow;
    connect(m_simulationThread, SIGNAL(newEvent(Event)),
            m_statisticsWindow, SLOT(newEvent(Event)));
}

void MainWindow::arrivalDistributionParamsChanged()
{
    m_simulation->changeArrivalDistribution(m_ui->arrivalDistributionParamsWidget->getDistributionParams());
}

void MainWindow::updateStationParams()
{
    int id = m_simulationScene->getSelectedStationId();
    if (id == -1)
    {
        m_ui->stationOptionsDockWidgetContents->setEnabled(false);
    }
    else
    {
        m_ui->stationOptionsDockWidgetContents->setEnabled(true);

        disconnectStationParamsWidgets();

        Station stationParams = m_simulation->getStation(id);

        if (stationParams.queueType == QueueType::Fifo)
        {
            m_ui->queueTypeComboBox->setCurrentIndex(0);
        }
        else if (stationParams.queueType == QueueType::Random)
        {
            m_ui->queueTypeComboBox->setCurrentIndex(1);
        }

        if (stationParams.queueLength == 0)
        {
            m_ui->infiniteQueueRadioButton->setChecked(true);
            m_ui->queueLengthSpinBox->setEnabled(false);
        }
        else
        {
            m_ui->finiteQueueRadioButton->setChecked(true);
            m_ui->queueLengthSpinBox->setEnabled(true);
            m_ui->queueLengthSpinBox->setValue(stationParams.queueLength);
        }

        m_ui->processorCountSpinBox->setValue(stationParams.processorCount);

        m_ui->processingDistributionParamsWidget->setDistributionParams(stationParams.serviceTimeDistribution);

        connectStationParamsWidgets();
    }
}

void MainWindow::stationParamsChanged()
{
    int id = m_simulationScene->getSelectedStationId();

    Station stationParams;

    if (m_ui->queueTypeComboBox->currentIndex() == 0)
    {
        stationParams.queueType = QueueType::Fifo;
    }
    else if (m_ui->queueTypeComboBox->currentIndex() == 1)
    {
        stationParams.queueType = QueueType::Random;
    }

    if (m_ui->infiniteQueueRadioButton->isChecked())
    {
        stationParams.queueLength = 0;
        m_ui->queueLengthSpinBox->setEnabled(false);
    }
    else
    {
        stationParams.queueLength = m_ui->queueLengthSpinBox->value();
        m_ui->queueLengthSpinBox->setEnabled(true);
    }

    stationParams.processorCount = m_ui->processorCountSpinBox->value();

    stationParams.serviceTimeDistribution = m_ui->processingDistributionParamsWidget->getDistributionParams();

    m_simulation->changeStation(id, stationParams);
    m_simulationScene->changeStation(id, stationParams);
}

void MainWindow::updateConnectionParams()
{
    auto selectedConnection = m_simulationScene->getSelectedConnection();

    if (selectedConnection.first == -1)
    {
        m_ui->connectionOptionsDockWidgetContents->setEnabled(false);
    }
    else
    {
        m_ui->connectionOptionsDockWidgetContents->setEnabled(true);

        disconnectConnectionParamsWidgets();

        int connectionWeight = m_simulation->getConnectionWeight(selectedConnection.first, selectedConnection.second);
        m_ui->connectionWeightSpinBox->setValue(connectionWeight);

        connectConnectionParamsWidgets();
    }
}

void MainWindow::connectionParamsChanged()
{
    auto selectedConnection = m_simulationScene->getSelectedConnection();

    int weight = m_ui->connectionWeightSpinBox->value();
    m_simulation->changeConnectionWeight(selectedConnection.first, selectedConnection.second, weight);
    m_simulationScene->changeConnectionWeight(selectedConnection.first, selectedConnection.second, weight);
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

