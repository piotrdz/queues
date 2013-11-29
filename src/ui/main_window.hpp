#pragma once

#include <QMainWindow>

namespace Ui
{
    class MainWindow;
}

class SimulationScene;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    virtual ~MainWindow();

private:
    Ui::MainWindow* m_ui;
    SimulationScene* m_simulationScene;
};

