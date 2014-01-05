#include "engine/simulation_check_helper.hpp"

#include <QDebug>
#include <QSet>
#include <QStack>


bool SimulationCheckHelper::check(const SimulationInstance& instance)
{
    int numberOfInputs = 0;
    int numberOfOutputs = 0;
    QSet<int> stationIds;
    for (const Station& station : instance.stations)
    {
        if (station.id == INVALID_STATION_ID)
        {
            qDebug() << "Check: invalid station id";
            return false;
        }

        if (station.id == INPUT_STATION_ID)
        {
            ++numberOfInputs;
        }
        else if (station.id == OUTPUT_STATION_ID)
        {
            ++numberOfOutputs;
        }
        else
        {
            if (station.queueLength < 0)
            {
                qDebug() << "Check: invalid queue length";
            }

            if (station.processorCount <= 0)
            {
                qDebug() << "Check: invalid number of processors";
            }
        }

        stationIds.insert(station.id);
    }

    if (stationIds.size() != instance.stations.size())
    {
        qDebug() << "Check: non-unique station ids";
        return false;
    }

    if (numberOfInputs != 1)
    {
        qDebug() << "Check: invalid number of inputs";
        return false;
    }

    if (numberOfOutputs != 1)
    {
        qDebug() << "Check: invalid number of outputs";
        return false;
    }

    QSet<QPair<int,int>> connectionPairs;
    for (const Connection& connection : instance.connections)
    {
        if (!stationIds.contains(connection.from) || !stationIds.contains(connection.to))
        {
            qDebug() << "Check: invalid connection ids";
            return false;
        }

        if (connection.weight <= 0)
        {
            qDebug() << "Check: invalid connection weight";
            return false;
        }

        if (connection.from == INPUT_STATION_ID && connection.to == OUTPUT_STATION_ID)
        {
            qDebug() << "Check: direct input -> output connection";
            return false;
        }

        if (connectionPairs.contains(qMakePair<int, int>(connection.from, connection.to)) ||
            connectionPairs.contains(qMakePair<int, int>(connection.to, connection.from)))
        {
            qDebug() << "Check: duplicate/two-way connection";
            return false;
        }

        connectionPairs.insert(qMakePair<int, int>(connection.from, connection.to));
    }

    for (const Station& station : instance.stations)
    {
        int numberOfIncomingConnections = 0;
        int numberOfOutgoingConnections = 0;
        for (const Connection& connection : instance.connections)
        {
            if (connection.to == station.id)
            {
                ++numberOfIncomingConnections;
            }

            if (connection.from == station.id)
            {
                ++numberOfOutgoingConnections;
            }
        }

        if (station.id == INPUT_STATION_ID)
        {
            if (numberOfIncomingConnections > 0 || numberOfOutgoingConnections == 0)
            {
                qDebug() << "Check: invalid number of connections for input station";
                return false;
            }
        }
        else if (station.id == OUTPUT_STATION_ID)
        {
            if (numberOfIncomingConnections == 0 || numberOfOutgoingConnections > 0)
            {
                qDebug() << "Check: invalid number of connections for output station";
                return false;
            }
        }
        else
        {
            if (numberOfIncomingConnections == 0 || numberOfOutgoingConnections == 0)
            {
                qDebug() << "Check: invalid number of connections for station";
                return false;
            }
        }
    }

    bool hasCycle = checkForCycles(instance.connections, INPUT_STATION_ID);
    if (hasCycle)
    {
        qDebug() << "Check: cycle detected";
        return false;
    }

    qDebug() << "Check: ok";
    return true;
}

bool SimulationCheckHelper::checkForCycles(const QList<Connection>& connections, int startStation)
{
    bool hasCycle = false;

    QMap<int, int> stationMarkings;
    QStack<Connection> connectionsToExplore;

    int currentStation = startStation;
    int connectedStationMarking = 0;
    while (!hasCycle)
    {
        int currentStationMarking = connectedStationMarking + 1;
        stationMarkings[currentStation] = currentStationMarking;

        for (const Connection& connection : connections)
        {
            if (connection.from == currentStation)
            {
                if (stationMarkings.contains(connection.to) && stationMarkings.value(connection.to) < currentStationMarking)
                {
                    hasCycle = true;
                    break;
                }
                else
                {
                    connectionsToExplore.push(connection);
                }
            }
        }

        if (connectionsToExplore.empty())
        {
            break;
        }

        Connection connection = connectionsToExplore.pop();
        connectedStationMarking = stationMarkings[connection.from];
        currentStation = connection.to;
    }

    return hasCycle;
}
