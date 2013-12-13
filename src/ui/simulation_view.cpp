#include "ui/simulation_view.hpp"

#include "ui/simulation_scene.hpp"

#include <QMouseEvent>


SimulationView::SimulationView(QWidget* parent)
 : QGraphicsView(parent)
 , m_simulationScene(nullptr)
 , m_processEventMode(false)
{
}

SimulationView::~SimulationView()
{
    m_simulationScene = nullptr;
}

void SimulationView::setSimulationScene(SimulationScene* simulationScene)
{
    m_simulationScene = simulationScene;
    setScene(simulationScene);
}

void SimulationView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton)
    {
        m_processEventMode = true;
        m_simulationScene->processMousePress(mapToScene(event->pos()));
        event->ignore();
        return;
    }

    QGraphicsView::mousePressEvent(event);
}

void SimulationView::mouseMoveEvent(QMouseEvent* event)
{
    if (m_processEventMode)
    {
        m_simulationScene->processMouseMove(mapToScene(event->pos()));
        event->ignore();
        return;
    }

    QGraphicsView::mouseMoveEvent(event);
}

void SimulationView::mouseReleaseEvent(QMouseEvent* event)
{
    if (m_processEventMode)
    {
        m_processEventMode = false;
        m_simulationScene->processMouseRelease(mapToScene(event->pos()));
        event->ignore();
        return;
    }

    QGraphicsView::mouseReleaseEvent(event);
}
