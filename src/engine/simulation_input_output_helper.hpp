#pragma once

#include "engine/simulation_instance.hpp"

#include <QString>

struct ParseContext;
class QTextStream;

class SimulationInputOutputHelper
{
public:
    static SimulationInstance readFromFile(const QString& path);
    static void saveToFile(const QString& path, const SimulationInstance& simulationInstance);

private:
    static bool parseLine(ParseContext& context);
    static bool parseFirstLine(ParseContext& context);
    static bool parseStation(const QString& line, Station& station, int& stationId);
    static bool parseConnection(const QString& line, Connection& connection);
    static bool parseDistribution(const QString& str, Distribution& distribution);

    static QString distributionToString(const Distribution& distribution);
    static void saveFirstLine(QTextStream& out, const SimulationInstance& simulationInstance);
    static void saveStation(QTextStream& out, const Station& station);
    static void saveConnection(QTextStream& out, const Connection& connection);
};
