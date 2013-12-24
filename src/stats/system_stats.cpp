#include "stats/system_stats.hpp"

SystemStat::SystemStat()
 : m_lastEventTime(0.0)
{}

bool SystemStat::update(Event event)
{
    if (event.type == EventType::TaskInput || event.type == EventType::TaskOutput)
    {
        updateSelf(event);

        m_lastEventTime = event.time;
        return true;
    }

    return false;
}

//////////////////////////////////////

SystemMeanTaskProcessingTimeStat::SystemMeanTaskProcessingTimeStat()
 : m_totalProcessingTime(0.0)
{}

void SystemMeanTaskProcessingTimeStat::reset()
{
    m_taskEntryTimes.clear();
    m_totalProcessingTime = 0.0;
    m_totalNumberOfTasks = 0.0;
}

void SystemMeanTaskProcessingTimeStat::updateSelf(Event event)
{
    if (event.type == EventType::TaskInput)
    {
        m_taskEntryTimes.insert(event.taskId, event.time);
    }
    else if (event.type == EventType::TaskOutput)
    {
        double entryTime = m_taskEntryTimes.value(event.taskId);
        m_taskEntryTimes.remove(event.taskId);
        m_totalProcessingTime += (event.time - entryTime);
        ++m_totalNumberOfTasks;
    }

    if (m_totalNumberOfTasks > 0)
    {
        m_value = m_totalProcessingTime / m_totalNumberOfTasks;
    }
    else
    {
        m_totalNumberOfTasks = 0;
    }
}

//////////////////////////////////////

SystemMeanNumberOfTasksStat::SystemMeanNumberOfTasksStat()
 : m_numberOfTasks(0)
{}

void SystemMeanNumberOfTasksStat::reset()
{
    m_taskCountTimes.clear();
    m_numberOfTasks = 0;
}

void SystemMeanNumberOfTasksStat::updateSelf(Event event)
{
    while (m_taskCountTimes.size() < m_numberOfTasks+1)
    {
        m_taskCountTimes.append(0.0);
    }

    double deltaTime = event.time - m_lastEventTime;
    m_taskCountTimes[m_numberOfTasks] += deltaTime;

    if (event.type == EventType::TaskInput)
    {
        ++m_numberOfTasks;
    }
    else if (event.type == EventType::TaskOutput)
    {
        --m_numberOfTasks;
    }

    m_value = 0.0;
    double total = 0.0;

    for (int i = 0; i < m_taskCountTimes.size(); ++i)
    {
        m_value += i * m_taskCountTimes[i];
        total += m_taskCountTimes[i];
    }

    if (total > 0.0)
    {
        m_value /= total;
    }
    else
    {
        m_value = 0;
    }
}
