#include "engine/simulation_input_output_helper.hpp"

#include <QFile>
#include <QTextStream>
#include <boost/concept_check.hpp>

struct ParseContext
{
    ParseContext(SimulationInstance& instance)
     : simulationInstance(instance) {}

    QString line;
    int lineNumber = 0;
    int stationsCount = 0;
    int stationId = 0;
    SimulationInstance& simulationInstance;
};


SimulationInstance SimulationInputOutputHelper::readFromFile(const QString& path)
{
    SimulationInstance simulationInstance;

    QFile file(path);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);

        ParseContext context(simulationInstance);
        context.line = in.readLine();

        while (!context.line.isNull())
        {
            context.lineNumber++;
            if (!parseLine(context))
            {
                break;
            }
            context.line = in.readLine();
        }
    }

    return simulationInstance;
}

bool SimulationInputOutputHelper::parseLine(ParseContext& context)
{
    bool result = false;
    if (context.lineNumber == 1)
    {
        result = parseFirstLine(context);
    }
    else if (context.lineNumber <= context.stationsCount + 1)
    {
        Station station;
        result = parseStation(context.line, station, context.stationId);
        if (result)
        {
            context.simulationInstance.stations.append(station);
        }
    }
    else
    {
        Connection connection;
        result = parseConnection(context.line, connection);
        if (result)
        {
            context.simulationInstance.connections.append(connection);
        }
    }

    return result;
}

bool SimulationInputOutputHelper::parseFirstLine(ParseContext& context)
{
    QStringList components = context.line.split(",");
    if (components.size() != 2)
    {
        return false;
    }

    if (!parseDistribution(components[0], context.simulationInstance.arrivalTimeDistribution))
    {
        return false;
    }

    bool ok = false;
    context.stationsCount = components[1].toInt(&ok);

    return ok;
}

bool SimulationInputOutputHelper::parseStation(const QString& line, Station& station, int& stationId)
{
    QStringList components = line.split(",");
    if (components.size() != 3 && components.size() != 6)
    {
        return false;
    }

    if (components.size() == 3)
    {
        if (components[0] == "WE")
        {
            station.id = INPUT_STATION_ID;
        }
        else if (components[0] == "WY")
        {
            station.id = OUTPUT_STATION_ID;
        }
        else
        {
            return false;
        }
    }
    else
    {
        station.id = ++stationId;

        if (!parseDistribution(components[0], station.serviceTimeDistribution))
        {
            return false;
        }

        bool ok = false;
        station.processorCount = components[1].toInt(&ok);
        if (!ok)
        {
            return false;
        }

        if (components[2] == "FIFO")
        {
            station.queueType = QueueType::Fifo;
        }
        else if (components[2] == "RANDOM")
        {
            station.queueType = QueueType::Random;
        }
        else
        {
            return false;
        }

        station.queueLength = components[3].toInt(&ok);
        if (!ok)
        {
            return false;
        }
    }

    bool ok = false;
    qreal x = components.at(components.size()-2).toFloat(&ok);
    if (!ok)
    {
        return false;
    }

    qreal y = components.at(components.size()-1).toFloat(&ok);
    if (!ok)
    {
        return false;
    }

    station.position = QPointF(x, y);

    return true;
}

bool SimulationInputOutputHelper::parseConnection(const QString& line, Connection& connection)
{
    QStringList components = line.split(",");
    if (components.size() != 3)
    {
        return false;
    }

    auto parseId = [](const QString& str, int &id) -> bool
    {
        bool ok = false;
        if (str == "WE")
        {
            id = INPUT_STATION_ID;
            ok = true;
        }
        else if (str == "WY")
        {
            id = OUTPUT_STATION_ID;
            ok = true;
        }
        else
        {
            id = str.toInt(&ok);
        }

        return ok;
    };

    if (!parseId(components[0], connection.from))
    {
        return false;
    }

    if (!parseId(components[1], connection.to))
    {
        return false;
    }

    bool ok = false;
    connection.weight = components[2].toInt(&ok);
    if (!ok)
    {
        return false;
    }

    return true;
}

bool SimulationInputOutputHelper::parseDistribution(const QString& str, Distribution& distribution)
{
    if (str.isEmpty())
    {
        return false;
    }

    QChar letter = str.at(0);
    QStringList components = str.mid(1).split("_");

    auto parseParams = [&distribution,&components](int numberOfParams) -> bool
    {
        if (components.size() != numberOfParams)
        {
            return false;
        }

        bool ok = false;
        distribution.param1 = components[0].toFloat(&ok);
        if (!ok)
        {
            return false;
        }

        if (numberOfParams > 1)
        {
            distribution.param2 = components[1].toFloat(&ok);
            if (!ok)
            {
                return false;
            }
        }

        return true;
    };

    switch (letter.toLatin1())
    {
        case 'C':
            distribution.type = DistributionType::Constant;
            parseParams(1);
            break;

        case 'U':
            distribution.type = DistributionType::Constant;
            parseParams(2);
            break;

        case 'E':
            distribution.type = DistributionType::Exponential;
            parseParams(1);
            break;

        case 'N':
            distribution.type = DistributionType::Normal;
            parseParams(2);
            break;

        default:
            break;
    }

    return true;
}

void SimulationInputOutputHelper::saveToFile(const QString& path, const SimulationInstance& simulationInstance)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return;
    }

    QTextStream out(&file);

    saveFirstLine(out, simulationInstance);

    for (const Station& station : simulationInstance.stations)
    {
        saveStation(out, station);
    }

    for (const Connection& connection : simulationInstance.connections)
    {
        saveConnection(out, connection);
    }
}

QString SimulationInputOutputHelper::distributionToString(const Distribution& distribution)
{
    QString str;
    QTextStream out(&str);

    auto printParams = [&out,&distribution](int numberOfParams)
    {
        out << distribution.param1;
        if (numberOfParams > 1)
        {
            out << '_' << distribution.param2;
        }
    };

    switch (distribution.type)
    {
        case DistributionType::Constant:
            out << 'C';
            printParams(1);
            break;

        case DistributionType::Uniform:
            out << 'U';
            printParams(2);
            break;

        case DistributionType::Exponential:
            out << 'E';
            printParams(1);
            break;

        case DistributionType::Normal:
            out << 'N';
            printParams(2);
            break;
    }

    return str;
}

void SimulationInputOutputHelper::saveFirstLine(QTextStream& out, const SimulationInstance& simulationInstance)
{
    out << distributionToString(simulationInstance.arrivalTimeDistribution);
    out << ",";
    out << simulationInstance.stations.size();
    out << "\n";
}

void SimulationInputOutputHelper::saveStation(QTextStream& out, const Station& station)
{
    if (station.id == INPUT_STATION_ID)
    {
        out << "WE";
    }
    else if (station.id == OUTPUT_STATION_ID)
    {
        out << "WY";
    }
    else
    {
        out << distributionToString(station.serviceTimeDistribution);
        out << ",";
        out << station.processorCount;
        out << ",";
        if (station.queueType == QueueType::Fifo)
        {
            out << "FIFO";
        }
        else
        {
            out << "RANDOM";
        }
        out << ",";
        out << station.queueLength;
    }
    out << ",";
    out << station.position.x();
    out << ",";
    out << station.position.y();
    out << "\n";
}

void SimulationInputOutputHelper::saveConnection(QTextStream& out, const Connection& connection)
{
    auto stationIdToString = [](int id) -> QString
    {
        QString result;
        if (id == INPUT_STATION_ID)
        {
            result = "WE";
        }
        else if (id == OUTPUT_STATION_ID)
        {
            result = "WY";
        }
        else
        {
            result = QString().setNum(id);
        }
        return result;
    };

    out << stationIdToString(connection.from);
    out << ",";
    out << stationIdToString(connection.to);
    out << ",";
    out << connection.weight;
    out << "\n";
}
