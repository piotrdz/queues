#pragma once

#include "engine/event.hpp"
#include "engine/simulation_instance.hpp"

#include <QList>

class Simulation
{
public:
    Simulation();

    void setInstance(const SimulationInstance& instance);
    const SimulationInstance& getInstance() const;

    void addStation(const Station& station);
    void addConnection(const Connection& connection);

    void changeArrivalDistribution(const Distribution& distribution);

    int getNextStationId() const;
    Station getStation(int id) const;
    void changeStation(int id, const StationParams& stationParams);

    int getConnectionWeight(int from, int to) const;
    void changeConnectionWeight(int from, int to, int weight);

    bool connectionExists(int from, int to) const;
    bool isConnectionPossible(int from, int to) const;

    void removeStation(int id);
    void removeConnection(int from, int to);

    void reset();
    Event simulateNextStep();
    double getCurrentTime();
    double getTimeToNextStep();

    static SimulationInstance readFromFile(std::string path);
    static void saveToFile(std::string path, const SimulationInstance& simulationInstance);
    static bool check(const SimulationInstance& instance);

private:
    double m_currentTime;
    SimulationInstance m_instance;
    int m_nextStationId;
};
