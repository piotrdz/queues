#include "engine/simulation.hpp"
#include <fstream>
#include <iostream>
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

void Simulation::addStation(const Station& station)
{
    m_instance.stations.append(station);
    m_nextStationId = std::max(m_nextStationId, station.id+1);
}

void Simulation::addConnection(const Connection& connection)
{
    m_instance.connections.append(connection);
}

bool Simulation::check()
{
    // TODO
    return true;
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

Event Simulation::simulateNextStep()
{
    m_currentTime += getTimeToNextStep();
    // TODO
    return Event();
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

void Simulation::readFromFile(std::string path)
{
   reset();
   SimulationInstance simulationInstance;
   std::string line;
   int lineNo = 0;
   int stationsNo = 0;
   
   std::ofstream confFile (path);
   if(confFile.is_open())
   {
       while (std::getline(confFile, line) )
       {
	 if (lineNo == 0)
	 {
	     unsigned comaPos = 0;
	     unsigned underlinePos = 0;
	     unsigned spacePos = 0;
	     
	     if (line[0] == "C")
	     {
	         comaPos = line.find(",");
		 simulationInstance.arrivalTimeDistribution.type = DistributionType::Constant;
		 simulationInstance.arrivalTimeDistribution.param1 = atof(line.substr(1, comaPos-1));
	     }
	     if (line[0] == "U")
	     {
	       underlinePos = line.find("_");
	       comaPos = line.find(",");
	       simulationInstance.arrivalTimeDistribution.type = DistributionType::Uniform;
	       simulationInstance.arrivalTimeDistribution.param1 = atof(line.substr(1, underlinePos-1));
	       simulationInstance.arrivalTimeDistribution.param2 = atof(line.substr(underlinePos+1, comaPos-underlinePos));
	     }
	     if (line[0] == "E")
	     {
	        comaPos = line.find(",");
		 simulationInstance.arrivalTimeDistribution.type = DistributionType::Exponential;
		 simulationInstance.arrivalTimeDistribution.param1 = atof(line.substr(1, comaPos-1));
	     }
	     if (line[0] == "N")
	     {
	       underlinePos = line.find("_");
	       comaPos = line.find(",");
	       simulationInstance.arrivalTimeDistribution.type = DistributionType::Normal;
	       simulationInstance.arrivalTimeDistribution.param1 = atof(line.substr(1, underlinePos-1));
	       simulationInstance.arrivalTimeDistribution.param2 = atof(line.substr(underlinePos+1, comaPos-underlinePos));
	     }
	     
	     spacePos = line.find(" ");
	     stationsNo = atoi(line.substr(spacePos+1, line.length()-spacePos));
	 }
	 
	 else if(lineNo <= stationsNo)
	 {
	     //add new station
	     Station newStation = Station();
	     loadDistribution(line, newStation);
	     
	     float posX = 0.0;
	     float posY = 0.0;
	     unsigned comaPos = line.find(",");
	     newStation.processorCount = atoi(line[comaPos+1]);
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
	     newStation.queueLength = atoi(line.substr(0, comaPos));
	     line = line.substr(comaPos+1);
	     comaPos = line.find(",");
	     posX = atof(line.substr(0, comaPos));
	     posY = atof(line.substr(comaPos+1, line.length()-comaPos));
	     QPointF point(posX, posY);
	     newStation.position = point;
	     
	     simulationInstance.stations.addStation(newStation);
	 }
	 else if(lineNo > stationsNo)
	 {
	      //add new connection
	      //0 - wejście z całego systemu
	      //stanowiska numerujemy od 1 do n
	      //n+1 - wyjście z całego systemu
	      Connection newCon = Connection();
	      unsigned comaPos = line.find(",");
	      if (line[0] == "W")
	      {
		  newCon.from = 0;
	      }
	      else
	      {
		  newCon.from = atoi(line.substr(0, comaPos);
	      }
	      line = line.substr(comaPos+1);
	      comaPos = line.find(",");
	      if (line[0] == "W")
	      {
		  newCon.to = stationsNo+1;
	      }
	      else
	      {
		  newCon.to = atoi(line.substr(0, comaPos);
	      }
	      newCon.weight = atoi(line.substr(comaPos+1, line.length()-comaPos));
	      
	      simulationInstance.connections.addConnection(newCon);	   
	 }
	 lineNo++;  
       }
       confFile.close();
   }
}

void Simulation::saveToFile()
{
    
  
}

void loadDistribution(std::string line, Station& station)
{
     unsigned comaPos = 0;
     unsigned underlinePos = 0;
	     
	     if (line[0] == "C")
	     {
	         comaPos = line.find(",");
		 station.serviceTimeDistribution.type = DistributionType::Constant;
		 station.serviceTimeDistribution.param1 = atof(line.substr(1, comaPos-1));
	     }
	     if (line[0] == "U")
	     {
	       underlinePos = line.find("_");
	       comaPos = line.find(",");
	       station.serviceTimeDistribution.type = DistributionType::Uniform;
	       station.serviceTimeDistribution.param1 = atof(line.substr(1, underlinePos-1));
	       station.serviceTimeDistribution.param2 = atof(line.substr(underlinePos+1, comaPos-underlinePos));
	     }
	     if (line[0] == "E")
	     {
	         comaPos = line.find(",");
		 station.serviceTimeDistribution.type = DistributionType::Exponential;
		 station.serviceTimeDistribution.param1 = atof(line.substr(1, comaPos-1));
	     }
	     if (line[0] == "N")
	     {
	       underlinePos = line.find("_");
	       comaPos = line.find(",");
	       station.serviceTimeDistribution.type = DistributionType::Normal;
	       station.serviceTimeDistribution.param1 = atof(line.substr(1, underlinePos-1));
	       station.serviceTimeDistribution.param2 = atof(line.substr(underlinePos+1, comaPos-underlinePos));
	     }
}
