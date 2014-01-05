#include "engine/simulation.hpp"

#include <QDebug>
#include <QPointF>
#include <QStack>
#include <QSet>

#include <fstream>
#include <iostream>
#include <sstream>


namespace rnd = boost::random;

QDebug operator<<(QDebug dbg, EventType eventType)
{
    QString eventName;
    switch (eventType)
    {
        case EventType::TaskInput:
            eventName = "TaskInput";
            break;

        case EventType::TaskOutput:
            eventName = "TaskOutput";
            break;

        case EventType::TaskAddedToQueue:
            eventName = "TaskAddedToQueue";
            break;

        case EventType::TaskStartedProcessing:
            eventName = "TaskStartedProcessing";
            break;

        case EventType::TaskEndedProcessing:
            eventName = "TaskEndedProcessing";
            break;

        case EventType::MachineIsIdle:
            eventName = "MachineIsIdle";
            break;

        case EventType::QueueHasPlace:
            eventName = "QueueHasPlace";
            break;
    }

    dbg.space() << eventName;
    return dbg.space();
}


Simulation::Simulation()
 : m_nextStationId(1)
 , m_nextTaskId(1)
 , m_currentTime(0.0)
{}

void Simulation::setInstance(const SimulationInstance& instance)
{
    m_instance = instance;

    m_nextStationId = 1;
    for (const Station& station : m_instance.stations)
    {
        m_nextStationId = std::max(m_nextStationId, station.id+1);
    }

    reset();
}

const SimulationInstance& Simulation::getInstance() const
{
    return m_instance;
}

void Simulation::addStation(const Station& station)
{
    m_instance.stations.append(station);
    m_nextStationId = std::max(m_nextStationId, station.id+1);

    StationState stationState;
    stationState.stationId = station.id;
    for (int i = 0; i < station.queueLength; ++i)
    {
        stationState.tasksInQueue.append(EMPTY_TASK_ID);
    }
    for (int i = 0; i < station.processorCount; ++i)
    {
        stationState.tasksInProcessors.append(EMPTY_TASK_ID);
    }
    m_stationStates.append(stationState);
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

    StationState& stationState = getStationState(id);
    stationState.tasksInQueue.clear();
    for (int i = 0; i < stationParams.queueLength; ++i)
    {
        stationState.tasksInQueue.append(EMPTY_TASK_ID);
    }
    stationState.tasksInProcessors.clear();
    for (int i = 0; i < stationParams.processorCount; ++i)
    {
        stationState.tasksInProcessors.append(EMPTY_TASK_ID);
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
        if (stationIt->id == id)
        {
            m_instance.stations.erase(stationIt);
            break;
        }
    }

    for (auto stationStateIt = m_stationStates.begin(); stationStateIt != m_stationStates.end(); ++stationStateIt)
    {
        if (stationStateIt->stationId == id)
        {
            m_stationStates.erase(stationStateIt);
            break;
        }
    }

    auto connectionIt = m_instance.connections.begin();
    while (connectionIt != m_instance.connections.end())
    {
        if (connectionIt->from == id || connectionIt->to == id)
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
    m_nextTaskId = 1;

    m_eventQueue.clear();

    Event initialTaskEvent;
    initialTaskEvent.type = EventType::TaskInput;
    initialTaskEvent.time = 0.0;
    initialTaskEvent.taskId = generateTaskId();
    m_eventQueue.enqueue(initialTaskEvent);

    m_stationStates.clear();

    for (const Station& station : m_instance.stations)
    {
        StationState stationState;
        stationState.stationId = station.id;
        for (int i = 0; i < station.queueLength; ++i)
        {
            stationState.tasksInQueue.append(EMPTY_TASK_ID);
        }
        for (int i = 0; i < station.processorCount; ++i)
        {
            stationState.tasksInProcessors.append(EMPTY_TASK_ID);
        }
        m_stationStates.append(stationState);
    }
}

double Simulation::getCurrentTime()
{
    return m_currentTime;
}

double Simulation::getTimeToNextStep()
{
    if (m_eventQueue.isEmpty())
    {
        return 0.0;
    }

    double nextEventTime = m_eventQueue.head().time;
    return nextEventTime - m_currentTime;
}

Event Simulation::simulateNextStep()
{
    Event event = m_eventQueue.dequeue();
    processEvent(event);
    return event;
}

void Simulation::debugDump()
{
    qDebug() << "currentTime:" << m_currentTime;
    qDebug() << "eventQueue:";

    QList<Event> allTasks = m_eventQueue.getAll();
    for (const Event& event : allTasks)
    {
        qDebug() << " time:" << event.time << ", type:" << event.type << ", stationId:" << event.stationId << ", taskId:" << event.taskId;
    }

    qDebug() << "stationStates:";
    for (const StationState& stationState : m_stationStates)
    {
        if (stationState.stationId == INPUT_STATION_ID || stationState.stationId == OUTPUT_STATION_ID)
        {
            continue;
        }

        qDebug() << " stationId:" << stationState.stationId << ",tasksInQueue:" << stationState.tasksInQueue << ",tasksInProcessors:" << stationState.tasksInProcessors;
    }
}

void Simulation::processEvent(Event event)
{
    m_currentTime = event.time;

    switch (event.type)
    {
        case EventType::TaskInput:
            processTaskInput(event);
            break;

        case EventType::TaskAddedToQueue:
            processTaskAddedToQueue(event);
            break;

        case EventType::TaskStartedProcessing:
            processTaskStartedProcessing(event);
            break;

        case EventType::TaskEndedProcessing:
            processTaskEndedProcessing(event);
            break;

        case EventType::QueueHasPlace:
            processTaskQueueHasPlace(event);
            break;

        case EventType::MachineIsIdle:
            processTaskMachineIsIdle(event);
            break;

        case EventType::TaskOutput:
            break;
    }
}

void Simulation::processTaskInput(Event event)
{
    QList<Connection> connections = getConnectionsFrom(INPUT_STATION_ID);
    Connection connectionToFollow = chooseConnectionToFollow(connections);

    if (connectionToFollow.from != INVALID_STATION_ID)
    {
        Event taskAddedToQueueEvent;
        taskAddedToQueueEvent.type = EventType::TaskAddedToQueue;
        taskAddedToQueueEvent.time = event.time;
        taskAddedToQueueEvent.taskId = event.taskId;
        taskAddedToQueueEvent.stationId = connectionToFollow.to;
        m_eventQueue.enqueue(taskAddedToQueueEvent);

        Event nextTaskEvent;
        nextTaskEvent.type = EventType::TaskInput;
        nextTaskEvent.time = event.time + generateTime(m_instance.arrivalTimeDistribution);
        nextTaskEvent.taskId = generateTaskId();
        m_eventQueue.enqueue(nextTaskEvent);
    }
}

void Simulation::processTaskAddedToQueue(Event event)
{
    Station station = getStation(event.stationId);
    StationState& stationState = getStationState(event.stationId);

    if (station.queueLength == 0)
    {
        stationState.tasksInQueue.append(event.taskId);
    }
    else
    {
        for (int& task : stationState.tasksInQueue)
        {
            if (task == EMPTY_TASK_ID)
            {
                task = event.taskId;
                break;
            }
        }
    }

    bool canBeProcessed = stationState.tasksInProcessors.contains(EMPTY_TASK_ID);

    if (canBeProcessed)
    {
        Event taskStartedProcessingEvent;
        taskStartedProcessingEvent.type = EventType::TaskStartedProcessing;
        taskStartedProcessingEvent.time = event.time;
        taskStartedProcessingEvent.taskId = event.taskId;
        taskStartedProcessingEvent.stationId = event.stationId;
        m_eventQueue.enqueue(taskStartedProcessingEvent);
    }
}

void Simulation::processTaskStartedProcessing(Event event)
{
    Station station = getStation(event.stationId);
    StationState& stationState = getStationState(event.stationId);

    if (station.queueLength == 0)
    {
        stationState.tasksInQueue.removeOne(event.taskId);
    }
    else
    {
        QList<int> newTasks;
        for (int task : stationState.tasksInQueue)
        {
            if (task != event.taskId)
            {
                newTasks.append(task);
            }
        }

        for (int i = 0; i < stationState.tasksInQueue.size(); ++i)
        {
            if (i < newTasks.size())
            {
                stationState.tasksInQueue[i] = newTasks[i];
            }
            else
            {
                stationState.tasksInQueue[i] = EMPTY_TASK_ID;
            }
        }
    }

    for (int& processorTask : stationState.tasksInProcessors)
    {
        if (processorTask == EMPTY_TASK_ID)
        {
            processorTask = event.taskId;
            break;
        }
    }

    Event taskQueueHasPlaceEvent;
    taskQueueHasPlaceEvent.type = EventType::QueueHasPlace;
    taskQueueHasPlaceEvent.time = event.time;
    taskQueueHasPlaceEvent.stationId = event.stationId;
    m_eventQueue.enqueue(taskQueueHasPlaceEvent);

    Event taskEndedProcessingEvent;
    taskEndedProcessingEvent.type = EventType::TaskEndedProcessing;
    taskEndedProcessingEvent.time = event.time + generateTime(station.serviceTimeDistribution);
    taskEndedProcessingEvent.taskId = event.taskId;
    taskEndedProcessingEvent.stationId = event.stationId;
    m_eventQueue.enqueue(taskEndedProcessingEvent);
}

void Simulation::processTaskQueueHasPlace(Event event)
{
    int finishedTaskFromConnectedStation = EMPTY_TASK_ID;
    int connectedStationId = INVALID_STATION_ID;

    QList<Connection> connections = getConnectionsTo(event.stationId);
    for (const Connection& connection : connections)
    {
        if (connection.from == INPUT_STATION_ID)
        {
            continue;
        }

        StationState& connectedStationState = getStationState(connection.from);

        for (int processorTask : connectedStationState.tasksInProcessors)
        {
            if (processorTask < 0)
            {
                finishedTaskFromConnectedStation = -processorTask;
                connectedStationId = connection.from;
                break;
            }
        }

        if (finishedTaskFromConnectedStation != EMPTY_TASK_ID)
        {
            break;
        }
    }

    if (finishedTaskFromConnectedStation != EMPTY_TASK_ID)
    {
        Event machineIsIdleEvent;
        machineIsIdleEvent.type = EventType::MachineIsIdle;
        machineIsIdleEvent.time = event.time;
        machineIsIdleEvent.stationId = connectedStationId;
        m_eventQueue.enqueue(machineIsIdleEvent);

        Event taskAddedToQueueEvent;
        taskAddedToQueueEvent.type = EventType::TaskAddedToQueue;
        taskAddedToQueueEvent.time = event.time;
        taskAddedToQueueEvent.taskId = event.taskId;
        taskAddedToQueueEvent.stationId = event.stationId;
        m_eventQueue.enqueue(machineIsIdleEvent);
    }
}

void Simulation::processTaskEndedProcessing(Event event)
{
    StationState& stationState = getStationState(event.stationId);

    for (int& processorTask : stationState.tasksInProcessors)
    {
        if (processorTask == event.taskId)
        {
            processorTask = -event.taskId;
            break;
        }
    }

    QList<Connection> connections = getConnectionsFrom(event.stationId);
    Connection connection = chooseConnectionToFollow(connections);

    if (connection.from != INVALID_STATION_ID)
    {
        Connection connection = chooseConnectionToFollow(connections);

        Event taskMachineIsIdleEvent;
        taskMachineIsIdleEvent.type = EventType::MachineIsIdle;
        taskMachineIsIdleEvent.time = event.time;
        taskMachineIsIdleEvent.taskId = event.taskId;
        taskMachineIsIdleEvent.stationId = event.stationId;
        m_eventQueue.enqueue(taskMachineIsIdleEvent);

        if (connection.to == OUTPUT_STATION_ID)
        {
            Event taskOutputEvent;
            taskOutputEvent.type = EventType::TaskOutput;
            taskOutputEvent.time = event.time;
            taskOutputEvent.taskId = event.taskId;
            m_eventQueue.enqueue(taskOutputEvent);
        }
        else
        {
            Event taskAddedToQueueEvent;
            taskAddedToQueueEvent.type = EventType::TaskAddedToQueue;
            taskAddedToQueueEvent.time = event.time;
            taskAddedToQueueEvent.taskId = event.taskId;
            taskAddedToQueueEvent.stationId = connection.to;
            m_eventQueue.enqueue(taskAddedToQueueEvent);
        }
    }
}

void Simulation::processTaskMachineIsIdle(Event event)
{
    Station station = getStation(event.stationId);
    StationState& stationState = getStationState(event.stationId);

    for (int& processorTask : stationState.tasksInProcessors)
    {
        if (processorTask == -event.taskId)
        {
            processorTask = EMPTY_TASK_ID;
            break;
        }
    }

    int nextTaskToBeProcessed = EMPTY_TASK_ID;
    if (station.queueType == QueueType::Fifo)
    {
        nextTaskToBeProcessed = stationState.tasksInQueue.front();
    }
    else if (station.queueType == QueueType::Random)
    {
        nextTaskToBeProcessed = chooseRandomTaskFromQueue(stationState.tasksInQueue);
    }

    if (nextTaskToBeProcessed == EMPTY_TASK_ID)
    {
        return;
    }

    Event taskStartedProcessingEvent;
    taskStartedProcessingEvent.type = EventType::TaskStartedProcessing;
    taskStartedProcessingEvent.time = event.time;
    taskStartedProcessingEvent.taskId = nextTaskToBeProcessed;
    taskStartedProcessingEvent.stationId = event.stationId;
    m_eventQueue.enqueue(taskStartedProcessingEvent);
}

int Simulation::generateTaskId()
{
    return m_nextTaskId++;
}

QList<Connection> Simulation::getConnectionsFrom(int stationId) const
{
    QList<Connection> connections;
    for (const Connection& connection : m_instance.connections)
    {
        if (connection.from == stationId)
        {
            connections.append(connection);
        }
    }
    return connections;
}

QList<Connection> Simulation::getConnectionsTo(int stationId) const
{
    QList<Connection> connections;
    for (const Connection& connection : m_instance.connections)
    {
        if (connection.to == stationId)
        {
            connections.append(connection);
        }
    }
    return connections;
}

Simulation::StationState& Simulation::getStationState(int stationId)
{
    for (StationState& stationState : m_stationStates)
    {
        if (stationState.stationId == stationId)
        {
            return stationState;
        }
    }

    qFatal("Station state for stationId=%d not found", stationId);

    static StationState failStationState;
    return failStationState;
}

double Simulation::generateTime(const Distribution& distribution)
{
    double value = 1.0;

    switch (distribution.type)
    {
        case DistributionType::Constant:
        {
            value = distribution.param1;
            break;
        }

        case DistributionType::Uniform:
        {
            auto uniformDistribution = rnd::uniform_real_distribution<double>(distribution.param1, distribution.param2);
            value = uniformDistribution(m_randomGenerator);
            break;
        }

        case DistributionType::Normal:
        {
            auto normalDistribution = rnd::normal_distribution<double>(distribution.param1, distribution.param2);
            value = normalDistribution(m_randomGenerator);
            break;
        }

        case DistributionType::Exponential:
        {
            auto exponentialDistribution = rnd::exponential_distribution<double>(1.0 / distribution.param1);
            value = exponentialDistribution(m_randomGenerator);
            break;
        }
    }

    return value;
}

Connection Simulation::chooseConnectionToFollow(const QList<Connection>& connections)
{
    QList<Connection> possibleConnections;
    for (const Connection& connection : connections)
    {
        Station connectedStation = getStation(connection.to);
        StationState& connectedStationState = getStationState(connection.to);

        bool hasPlace = true;

        if (connectedStation.queueLength > 0)
        {
            hasPlace = connectedStationState.tasksInQueue.contains(EMPTY_TASK_ID);
        }

        if (hasPlace)
        {
            possibleConnections.append(connection);
        }
    }

    if (possibleConnections.isEmpty())
    {
        return Connection();
    }

    int totalWeightSum = 0;
    for (const Connection& connection : possibleConnections)
    {
        totalWeightSum += connection.weight;
    }

    auto distribution = rnd::uniform_int_distribution<int>(0, totalWeightSum);
    int randomWeightSum = distribution(m_randomGenerator);

    int weightSum = 0;
    int index = 0;
    for (int i = 0; i < possibleConnections.size(); ++i)
    {
        weightSum += possibleConnections.at(i).weight;
        if (weightSum >= randomWeightSum)
        {
            index = i;
            break;
        }
    }

    return possibleConnections.at(index);
}

int Simulation::chooseRandomTaskFromQueue(const QList<int>& tasks)
{
    QList<int> nonEmptyTasks;
    for (int taskId : tasks)
    {
        if (taskId != EMPTY_TASK_ID)
        {
            nonEmptyTasks.append(taskId);
        }
    }

    if (nonEmptyTasks.isEmpty())
    {
        return EMPTY_TASK_ID;
    }

    auto distribution = rnd::uniform_int_distribution<int>(0, nonEmptyTasks.size() - 1);

    int index = distribution(m_randomGenerator);

    return nonEmptyTasks.at(index);
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

