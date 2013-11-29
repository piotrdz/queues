#pragma once

#include <QGraphicsScene>

class SimulationScene : public QGraphicsScene
{
public:
    explicit SimulationScene(QObject* parent);
    virtual ~SimulationScene();
};
