#include "engine/simulation.hpp"

#include "engine/simulation_check_helper.hpp"
#include "engine/simulation_input_output_helper.hpp"

#include <QDebug>
#include <QPointF>
#include <QStack>
#include <QSet>

namespace rnd = boost::random;


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

    bool hasCycle = SimulationCheckHelper::checkForCycles(newConnections, from);

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
    return SimulationCheckHelper::check(m_instance);
}

bool Simulation::check(const SimulationInstance& instance)
{
    return SimulationCheckHelper::check(instance);
}

SimulationInstance Simulation::readFromFile(std::string path)
{
    return SimulationInputOutputHelper::readFromFile(path);
}

void Simulation::saveToFile(std::string path, const SimulationInstance& simulationInstance)
{
    return SimulationInputOutputHelper::saveToFile(path, simulationInstance);
}
