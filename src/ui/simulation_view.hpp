#pragma once

#include <QGraphicsView>

class SimulationScene;

class SimulationView : public QGraphicsView
{
public:
    explicit SimulationView(QWidget* parent);
    virtual ~SimulationView();

    void setSimulationScene(SimulationScene* simulationScene);

protected:
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);

private:
    SimulationScene* m_simulationScene;
    bool m_processEventMode;
};
