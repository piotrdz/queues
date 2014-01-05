#pragma once

#include "engine/connection.hpp"
#include "engine/simulation_instance.hpp"

#include <QList>

class SimulationCheckHelper
{
public:
    static bool check(const SimulationInstance& instance);
    static bool checkForCycles(const QList<Connection>& connections, int startStation);
};
