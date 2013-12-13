#pragma once

#include "engine/connection.hpp"
#include "engine/distribution.hpp"
#include "engine/station.hpp"

#include <QList>

struct SimulationInstance
{
    Distribution arrivalTimeDistribution;
    QList<Station> stations;
    QList<Connection> connections;
};
