#include "stats/station_stats.hpp"

StationStat::StationStat(int stationId)
 : m_stationId(stationId)
 , m_lastEventTime(0.0)
{
}

bool StationStat::update(Event event)
{
    if (event.stationId == m_stationId)
    {
        updateSelf(event);

        m_lastEventTime = event.time;

        return true;
    }

    return false;
}

//////////////////////////////////////

StationMeanUtilizedProcessorsStat::StationMeanUtilizedProcessorsStat(int stationId)
 : StationStat(stationId)
 , m_numberOfUtilizedProcessors(0)
{}

void StationMeanUtilizedProcessorsStat::reset()
{
    m_processorUtilizationTimes.clear();
    m_numberOfUtilizedProcessors = 0;
}

void StationMeanUtilizedProcessorsStat::updateSelf(Event event)
{
    while (m_processorUtilizationTimes.size() < m_numberOfUtilizedProcessors+1)
    {
        m_processorUtilizationTimes.append(0.0);
    }

    double deltaTime = event.time - m_lastEventTime;
    m_processorUtilizationTimes[m_numberOfUtilizedProcessors] += deltaTime;

    if (event.type == EventType::TaskStartedProcessing)
    {
        ++m_numberOfUtilizedProcessors;
    }
    else if (event.type == EventType::TaskEndedProcessing)
    {
        --m_numberOfUtilizedProcessors;
    }

    m_value = 0.0;
    double total = 0.0;

    for (int i = 0; i < m_processorUtilizationTimes.size(); ++i)
    {
        m_value += i * m_processorUtilizationTimes[i];
        total += m_processorUtilizationTimes[i];
    }

    if (total > 0.0)
    {
        m_value /= total;
    }
    else
    {
        m_value = 0.0;
    }
}

//////////////////////////////////////

StationMeanQueueLengthStat::StationMeanQueueLengthStat(int stationId)
 : StationStat(stationId)
 , m_numberOfTasksInQueue(0)
{}

void StationMeanQueueLengthStat::reset()
{
    m_queueLengthTimes.clear();
    m_numberOfTasksInQueue = 0;
}

void StationMeanQueueLengthStat::updateSelf(Event event)
{
    while (m_queueLengthTimes.size() < m_numberOfTasksInQueue+1)
    {
        m_queueLengthTimes.append(0.0);
    }

    double deltaTime = event.time - m_lastEventTime;
    m_queueLengthTimes[m_numberOfTasksInQueue] += deltaTime;

    if (event.type == EventType::TaskAddedToQueue)
    {
        ++m_numberOfTasksInQueue;
    }
    else if (event.type == EventType::TaskStartedProcessing)
    {
        --m_numberOfTasksInQueue;
    }

    m_value = 0.0;
    double total = 0.0;

    for (int i = 0; i < m_queueLengthTimes.size(); ++i)
    {
        m_value += i * m_queueLengthTimes[i];
        total += m_queueLengthTimes[i];
    }

    if (total > 0.0)
    {
        m_value /= total;
    }
    else
    {
        m_value = 0.0;
    }
}

//////////////////////////////////////

StationMeanWaitTimeStat::StationMeanWaitTimeStat(int stationId)
 : StationStat(stationId)
 , m_numberOfProcessedTasks(0)
 , m_totalWaitTime(0.0)
{}

void StationMeanWaitTimeStat::reset()
{
    m_taskEntryTimes.clear();
    m_numberOfProcessedTasks = 0;
    m_totalWaitTime = 0.0;
}

void StationMeanWaitTimeStat::updateSelf(Event event)
{
    if (event.type == EventType::TaskAddedToQueue)
    {
        m_taskEntryTimes.insert(event.taskId, event.time);
    }
    else if (event.type == EventType::TaskStartedProcessing)
    {
        ++m_numberOfProcessedTasks;
        double entryTime = m_taskEntryTimes.value(event.taskId);
        m_totalWaitTime += (event.time - entryTime);
        m_taskEntryTimes.remove(event.taskId);
    }

    if (m_numberOfProcessedTasks > 0)
    {
        m_value = m_totalWaitTime / m_numberOfProcessedTasks;
    }
    else
    {
        m_value = 0.0;
    }
}
