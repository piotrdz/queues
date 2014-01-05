#pragma once

#include "engine/simulation_instance.hpp"

class SimulationInputOutputHelper
{
public:
    static SimulationInstance readFromFile(std::string path);
    static void saveToFile(std::string path, const SimulationInstance& simulationInstance);

private:
    static void loadDistribution(std::string line, Station& station);
    static const char typeToString(DistributionType type);
};
