#include "engine/simulation_input_output_helper.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

void SimulationInputOutputHelper::loadDistribution(std::string line, Station& station)
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

const char SimulationInputOutputHelper::typeToString(DistributionType type)
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

SimulationInstance SimulationInputOutputHelper::readFromFile(std::string path)
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

void SimulationInputOutputHelper::saveToFile(std::string path, const SimulationInstance& simInstance)
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
