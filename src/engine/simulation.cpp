#include "engine/simulation.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <QPointF>


Simulation::Simulation()
 : m_currentTime(0.0)
 , m_nextStationId(0)
{}

void Simulation::setInstance(const SimulationInstance& instance)
{
    m_instance = instance;

    m_nextStationId = 0;
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
    // TODO cykle...
    return !connectionExists(from, to) && !connectionExists(to, from);
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

bool Simulation::check(const SimulationInstance& instance)
{
    // TODO
    return true;
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
                newStation.id = ++stationId;
                loadDistribution(line, newStation);

                float posX = 0.0;
                float posY = 0.0;
                unsigned comaPos = line.find(",");
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
                comaPos = line.find(",");
                posX = atof(line.substr(0, comaPos).c_str());
                posY = atof(line.substr(comaPos+1, line.length()-comaPos).c_str());
                QPointF point(posX, posY);
                newStation.position = point;

                simulationInstance.stations.append(newStation);
            }
            else if(lineNo > stationsNo)
            {
                //add new connection
                //1 - wejście do całego systemu
                //stanowiska numerujemy od 1 do n
                //n - wyjście z całego systemu
                Connection newCon = Connection();
                unsigned comaPos = line.find(",");
                if (line[0] == 'W')
                {
                    newCon.from = 1;
                }
                else
                {
                    newCon.from = atoi(line.substr(0, comaPos).c_str());
                }
                line = line.substr(comaPos+1);
                comaPos = line.find(",");
                if (line[0] == 'W')
                {
                    newCon.to = stationsNo;
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
            const int stationsNo = simInstance.stations.size();
            line.clear();

            if (simInstance.connections[i].from == 1)
            {
                line.append("WE,");
            }
            else
            {
                strStream1.str("");
                strStream1 << simInstance.connections[i].from;
                line.append(strStream1.str() + ",");
            }
            if (simInstance.connections[i].to == stationsNo)
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

