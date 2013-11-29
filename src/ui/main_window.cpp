#include "ui/main_window.hpp"

#include "ui/connection_item.hpp"
#include "ui/simulation_scene.hpp"
#include "ui/station_item.hpp"

#include "ui_main_window.h"


MainWindow::MainWindow()
 : m_ui(new Ui::MainWindow())
{
    m_ui->setupUi(this);

    m_simulationScene = new SimulationScene(this);

    m_ui->simulationView->setScene(m_simulationScene);

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
}

MainWindow::~MainWindow()
{
    delete m_ui;
    m_ui = nullptr;

    delete m_simulationScene;
    m_simulationScene = nullptr;
}
