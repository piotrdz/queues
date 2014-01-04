#include "engine/simulation.hpp"

#include <QDebug>
#include <QPointF>
#include <QStack>
#include <QSet>

#include <fstream>
#include <iostream>
#include <sstream>


Simulation::Simulation()
 : m_currentTime(0.0)
 , m_nextStationId(1)
{}

void Simulation::setInstance(const SimulationInstance& instance)
{
    m_instance = instance;

    m_nextStationId = 1;
    for (const Station& station : m_instance.stations)
    {
        m_nextStationId = std::max(m_nextStationId, station.id+1);
    }
}

const SimulationInstance& Simulation::getInstance() const
{
    return m_instance;
}

void Simulation::addStation(const Station& station)
{
    m_instance.stations.append(station);
    m_nextStationId = std::max(m_nextStationId, station.id+1);
}

void Simulation::addConnection(const Connection& connection)
{
    m_instance.connections.append(connection);
}

void Simulation::changeArrivalDistribution(const Distribution& distribution)
{
    m_instance.arrivalTimeDistribution = distribution;
}

int Simulation::getNextStationId() const
{
    return m_nextStationId;
}

Station Simulation::getStation(int id) const
{
    for (const Station& station : m_instance.stations)
    {
        if (station.id == id)
        {
            return station;
        }
    }

    return Station();
}

void Simulation::changeStation(int id, const StationParams& stationParams)
{
    for (Station& station : m_instance.stations)
    {
        if (station.id == id)
        {
            station.setParams(stationParams);
        }
    }
}

int Simulation::getConnectionWeight(int from, int to) const
{
    for (const Connection& connection : m_instance.connections)
    {
        if (connection.from == from && connection.to == to)
        {
            return connection.weight;
        }
    }

    return 0;
}

void Simulation::changeConnectionWeight(int from, int to, int weight)
{
    for (Connection& connection : m_instance.connections)
    {
        if (connection.from == from && connection.to == to)
        {
            connection.weight = weight;
        }
    }
}

bool Simulation::connectionExists(int from, int to) const
{
    for (const Connection& connection : m_instance.connections)
    {
        if (connection.from == from && connection.to == to)
        {
            return true;
        }
    }

    return false;
}

void Simulation::removeStation(int id)
{
    for (auto stationIt = m_instance.stations.begin(); stationIt != m_instance.stations.end(); ++stationIt)
    {
        if ((*stationIt).id == id)
        {
            m_instance.stations.erase(stationIt);
            break;
        }
    }

    auto connectionIt = m_instance.connections.begin();
    while (connectionIt != m_instance.connections.end())
    {
        if ((*connectionIt).from == id || (*connectionIt).to == id)
        {
            m_instance.connections.erase(connectionIt);
            break;
        }
        else
        {
            ++connectionIt;
        }
    }
}

void Simulation::removeConnection(int from, int to)
{
    for (auto connectionIt = m_instance.connections.begin(); connectionIt != m_instance.connections.end(); ++connectionIt)
    {
        if ((*connectionIt).from == from && (*connectionIt).to == to)
        {
            m_instance.connections.erase(connectionIt);
            break;
        }
    }
}

bool Simulation::isConnectionPossible(int from, int to) const
{
    if (to == INPUT_STATION_ID || from == OUTPUT_STATION_ID)
    {
        return false;
    }

    if (connectionExists(from, to) || connectionExists(to, from))
    {
        return false;
    }

    QList<Connection> newConnections = m_instance.connections;

    Connection newConnection;
    newConnection.from = from;
    newConnection.to = to;
    newConnections.append(newConnection);

    bool hasCycle = checkForCycles(newConnections, from);

    return !hasCycle;
}

void Simulation::reset()
{
    m_currentTime = 0.0;
    // TODO
}

// TODO: kod testowy, wywalić później
Event Simulation::demoEvent()
{
    static const QList<Event> events =
    {
        Event(EventType::TaskInput,             0.5, 0, 1),
        Event(EventType::TaskAddedToQueue,      1.0, 1, 1),
        Event(EventType::TaskStartedProcessing, 1.5, 1, 1),
        Event(EventType::TaskInput,             2.0, 0, 2),
        Event(EventType::TaskAddedToQueue,      2.5, 1, 2),
        Event(EventType::TaskEndedProcessing,   3.0, 1, 1),
        Event(EventType::MachineIsIdle,         3.5, 1, 1),
        Event(EventType::TaskAddedToQueue,      4.0, 2, 1),
        Event(EventType::TaskStartedProcessing, 4.5, 2, 1),
        Event(EventType::TaskStartedProcessing, 5.0, 1, 2),
        Event(EventType::TaskEndedProcessing,   5.5, 2, 1),
        Event(EventType::MachineIsIdle,         6.0, 2, 1),
        Event(EventType::TaskOutput,            6.5, 0, 1),
        Event(EventType::TaskEndedProcessing,   7.0, 1, 2),
        Event(EventType::MachineIsIdle,         7.5, 1, 2),
        Event(EventType::TaskAddedToQueue,      8.0, 2, 2),
        Event(EventType::TaskStartedProcessing, 8.5, 2, 2),
        Event(EventType::TaskEndedProcessing,   9.0, 2, 2),
        Event(EventType::MachineIsIdle,         9.5, 2, 2),
        Event(EventType::TaskOutput,           10.0, 0, 2)
    };

    double t = m_currentTime + 0.001;
    int index = static_cast<int>(std::floor(t / 0.5));
    if (index >= events.size())
    {
        return Event();
    }
    m_currentTime += 0.5;
    return events[index];
}

Event Simulation::simulateNextStep()
{
    // TODO: kod testowy, wywalić później
    return demoEvent();

    //m_currentTime += getTimeToNextStep();
    // TODO
    //return Event();
}

double Simulation::getCurrentTime()
{
    return m_currentTime;
}

double Simulation::getTimeToNextStep()
{
    // TODO
    return 0.5;
}

bool Simulation::check() const
{
    return Simulation::check(m_instance);
}

bool Simulation::check(const SimulationInstance& instance)
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

    return true;
}

bool Simulation::checkForCycles(const QList<Connection>& connections, int startStation)
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

void Simulation::loadDistribution(std::string line, Station& station)
{
    unsigned comaPos = 0;
    unsigned underlinePos = 0;

    if (line[0] == 'C')
    {
        comaPos = line.find(",");
        station.serviceTimeDistribution.type = DistributionType::Constant;
        station.serviceTimeDistribution.param1 = atof(line.substr(1, comaPos-1).c_str());
    }
    if (line[0] == 'U')
    {
        underlinePos = line.find("_");
        comaPos = line.find(",");
        station.serviceTimeDistribution.type = DistributionType::Uniform;
        station.serviceTimeDistribution.param1 = atof(line.substr(1, underlinePos-1).c_str());
        station.serviceTimeDistribution.param2 = atof(line.substr(underlinePos+1, comaPos-underlinePos).c_str());
    }
    if (line[0] == 'E')
    {
        comaPos = line.find(",");
        station.serviceTimeDistribution.type = DistributionType::Exponential;
        station.serviceTimeDistribution.param1 = atof(line.substr(1, comaPos-1).c_str());
    }
    if (line[0] == 'N')
    {
        underlinePos = line.find("_");
        comaPos = line.find(",");
        station.serviceTimeDistribution.type = DistributionType::Normal;
        station.serviceTimeDistribution.param1 = atof(line.substr(1, underlinePos-1).c_str());
        station.serviceTimeDistribution.param2 = atof(line.substr(underlinePos+1, comaPos-underlinePos).c_str());
    }
}

const char Simulation::typeToString(DistributionType type)
{
    switch (type)
    {
        case DistributionType::Uniform: return 'U';
        case DistributionType::Normal: return 'N';
        case DistributionType::Exponential: return 'E';
        case DistributionType::Constant: return 'C';
    }
    return 'C';
}

SimulationInstance Simulation::readFromFile(std::string path)
{
    SimulationInstance simulationInstance;
    std::string line;
    int lineNo = 0;
    int stationsNo = 0;
    int stationId = 0;

    std::ifstream confFile (path);
    if(confFile.is_open())
    {
        while (std::getline(confFile, line) )
        {
            if (lineNo == 0)
            {
                unsigned comaPos = 0;
                unsigned underlinePos = 0;

                if (line[0] == 'C')
                {
                    comaPos = line.find(",");
                    simulationInstance.arrivalTimeDistribution.type = DistributionType::Constant;
                    simulationInstance.arrivalTimeDistribution.param1 = atof(line.substr(1, comaPos-1).c_str());
                }
                if (line[0] == 'U')
                {
                    underlinePos = line.find("_");
                    comaPos = line.find(",");
                    simulationInstance.arrivalTimeDistribution.type = DistributionType::Uniform;
                    simulationInstance.arrivalTimeDistribution.param1 = atof(line.substr(1, underlinePos-1).c_str());
                    simulationInstance.arrivalTimeDistribution.param2 = atof(line.substr(underlinePos+1, comaPos-underlinePos).c_str());
                }
                if (line[0] == 'E')
                {
                    comaPos = line.find(",");
                    simulationInstance.arrivalTimeDistribution.type = DistributionType::Exponential;
                    simulationInstance.arrivalTimeDistribution.param1 = atof(line.substr(1, comaPos-1).c_str());
                }
                if (line[0] == 'N')
                {
                    underlinePos = line.find("_");
                    comaPos = line.find(",");
                    simulationInstance.arrivalTimeDistribution.type = DistributionType::Normal;
                    simulationInstance.arrivalTimeDistribution.param1 = atof(line.substr(1, underlinePos-1).c_str());
                    simulationInstance.arrivalTimeDistribution.param2 = atof(line.substr(underlinePos+1, comaPos-underlinePos).c_str());
                }

                comaPos = line.find(",");
                stationsNo = atoi(line.substr(comaPos+1, line.length()-comaPos).c_str());
            }

            else if(lineNo <= stationsNo)
            {
                //add new station
                Station newStation = Station();

                unsigned comaPos = 0;

                if (line.substr(0, 2) == "WE")
                {
                    newStation.id = INPUT_STATION_ID;
                    line = line.substr(3);
                }
                else if (line.substr(0, 2) == "WY")
                {
                    newStation.id = OUTPUT_STATION_ID;
                    line = line.substr(3);
                }
                else
                {
                    newStation.id = ++stationId;
                    loadDistribution(line, newStation);

                    comaPos = line.find(",");
                    newStation.processorCount = atoi(line.substr(comaPos+1,1).c_str());
                    if (line.substr(comaPos+3, 4) == "FIFO")
                    {
                        newStation.queueType = QueueType::Fifo;
                    }
                    else
                    {
                        newStation.queueType = QueueType::Random;
                    }
                    line = line.substr(comaPos+8);
                    comaPos = line.find(",");
                    newStation.queueLength = atoi(line.substr(0, comaPos).c_str());
                    line = line.substr(comaPos+1);
                }

                comaPos = line.find(",");

                float posX = 0.0;
                float posY = 0.0;
                posX = atof(line.substr(0, comaPos).c_str());
                posY = atof(line.substr(comaPos+1, line.length()-comaPos).c_str());
                QPointF point(posX, posY);
                newStation.position = point;

                simulationInstance.stations.append(newStation);
            }
            else if(lineNo > stationsNo)
            {
                //add new connection
                Connection newCon = Connection();
                unsigned comaPos = line.find(",");
                if (line[0] == 'W')
                {
                    newCon.from = INPUT_STATION_ID;
                }
                else
                {
                    newCon.from = atoi(line.substr(0, comaPos).c_str());
                }
                line = line.substr(comaPos+1);
                comaPos = line.find(",");
                if (line[0] == 'W')
                {
                    newCon.to = OUTPUT_STATION_ID;
                }
                else
                {
                    newCon.to = atoi(line.substr(0, comaPos).c_str());
                }
                newCon.weight = atoi(line.substr(comaPos+1, line.length()-comaPos).c_str());

                simulationInstance.connections.append(newCon);
            }
            lineNo++;
        }
        confFile.close();
   }
   return simulationInstance;
}

void Simulation::saveToFile(std::string path, const SimulationInstance& simInstance)
{
    std::ofstream confFile (path);
    std::string line;
    std::ostringstream strStream1, strStream2;

    if (confFile.is_open())
    {
        //first line
        strStream1 << simInstance.arrivalTimeDistribution.param1;
        if (simInstance.arrivalTimeDistribution.type == DistributionType::Constant)
        {
            line = "C" + strStream1.str();
        }
        else if (simInstance.arrivalTimeDistribution.type == DistributionType::Exponential)
        {
            line = "E" + strStream1.str();
        }
        else if (simInstance.arrivalTimeDistribution.type == DistributionType::Normal)
        {
            strStream2 << simInstance.arrivalTimeDistribution.param2;
            line = "N" + strStream1.str() + "_" + strStream2.str();
        }
        else if (simInstance.arrivalTimeDistribution.type == DistributionType::Uniform)
        {
            strStream2 << simInstance.arrivalTimeDistribution.param2;
            line = "U" + strStream1.str() + "_" + strStream2.str();
        }
        line.append(",");
        strStream1.str("");
        strStream1 << simInstance.stations.size();
        line.append(strStream1.str());
        line.append("\n");
        confFile << line;

        //opisy poszczególnych stacji

        for (int i=0; i< simInstance.stations.size(); i++)
        {
            line.clear();

            if (simInstance.stations[i].id == INPUT_STATION_ID)
            {
                line.append("WE");
            }
            else if (simInstance.stations[i].id == OUTPUT_STATION_ID)
            {
                line.append("WY");
            }
            else
            {
                line.append(1, typeToString(simInstance.stations[i].serviceTimeDistribution.type));
                strStream1.str("");
                strStream1 << simInstance.stations[i].serviceTimeDistribution.param1;
                line.append(strStream1.str());
                if(line[0] == 'N' || line[0] == 'U')
                {
                    strStream2.str("");
                    strStream2 << simInstance.stations[i].serviceTimeDistribution.param2;
                    line.append("_" + strStream2.str());
                }
                strStream1.str("");
                strStream1 << simInstance.stations[i].processorCount;
                line.append("," + strStream1.str());
                if (simInstance.stations[i].queueType == QueueType::Fifo)
                {
                    line.append(",FIFO");
                }
                else
                {
                    line.append(",RAND");
                }
                strStream1.str("");
                strStream1 << simInstance.stations[i].queueLength;
                line.append("," + strStream1.str());
            }

            strStream1.str("");
            strStream2.str("");
            strStream1 << simInstance.stations[i].position.x();
            strStream2 << simInstance.stations[i].position.y();
            line.append("," + strStream1.str() + "," + strStream2.str());

            line.append("\n");
            confFile << line;
        }

        for (int i=0; i < simInstance.connections.size(); i++)
        {
            line.clear();

            if (simInstance.connections[i].from == INPUT_STATION_ID)
            {
                line.append("WE,");
            }
            else
            {
                strStream1.str("");
                strStream1 << simInstance.connections[i].from;
                line.append(strStream1.str() + ",");
            }
            if (simInstance.connections[i].to == OUTPUT_STATION_ID)
            {
                line.append("WY,");
            }
            else
            {
                strStream1.str("");
                strStream1 << simInstance.connections[i].to;
                line.append(strStream1.str() + ",");
            }
            strStream1.str("");
            strStream1 << simInstance.connections[i].weight;
            line.append(strStream1.str() + "\n");

            confFile << line;
        }
        confFile.close();
    }
}

