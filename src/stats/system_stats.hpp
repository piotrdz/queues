#pragma once

#include "stats/stat.hpp"

#include <QHash>


class SystemStat : public Stat
{
public:
    SystemStat();

    virtual bool update(Event event) override;

protected:
    virtual void updateSelf(Event event) = 0;

protected:
    double m_lastEventTime;
};

//////////////////////////////////////

class SystemMeanTaskProcessingTimeStat : public SystemStat
{
public:
    SystemMeanTaskProcessingTimeStat();

    virtual void reset() override;

protected:
    virtual void updateSelf(Event event) override;

private:
    QHash<int, double> m_taskEntryTimes;
    double m_totalProcessingTime;
    int m_totalNumberOfTasks;
};

//////////////////////////////////////

class SystemMeanNumberOfTasksStat : public SystemStat
{
public:
    SystemMeanNumberOfTasksStat();

    virtual void reset() override;

protected:
    virtual void updateSelf(Event event) override;

private:
    QList<double> m_taskCountTimes;
    int m_numberOfTasks;
};
