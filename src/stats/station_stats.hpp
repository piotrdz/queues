#pragma once

#include "stats/stat.hpp"

#include <QHash>
#include <QList>


class StationStat : public Stat
{
public:
    explicit StationStat(int stationId);

    virtual bool update(Event event) override;

protected:
    virtual void updateSelf(Event event) = 0;

protected:
    const int m_stationId;
    double m_lastEventTime;
};

//////////////////////////////////////

class StationMeanUtilizedProcessorsStat : public StationStat
{
public:
    explicit StationMeanUtilizedProcessorsStat(int stationId);

    virtual void reset() override;

protected:
    virtual void updateSelf(Event event) override;

protected:
    QList<double> m_processorUtilizationTimes;
    int m_numberOfUtilizedProcessors;
};

//////////////////////////////////////

class StationMeanQueueLengthStat : public StationStat
{
public:
    explicit StationMeanQueueLengthStat(int stationId);

    virtual void reset() override;

protected:
    virtual void updateSelf(Event event) override;

private:
    QList<double> m_queueLengthTimes;
    int m_numberOfTasksInQueue;
};

//////////////////////////////////////

class StationMeanWaitTimeStat : public StationStat
{
public:
    explicit StationMeanWaitTimeStat(int stationId);

    virtual void reset() override;

protected:
    virtual void updateSelf(Event event) override;

private:
    QHash<int, double> m_taskEntryTimes;
    int m_numberOfProcessedTasks;
    double m_totalWaitTime;
};
