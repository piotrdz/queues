#pragma once

#include "engine/distribution.hpp"

#include <QPointF>

enum class QueueType
{
    Fifo,
    Random
};

struct StationParams
{
    QueueType queueType;
    int queueLength;
    int processorCount;
    Distribution serviceTimeDistribution;
    QPointF position;

    StationParams()
     : queueType(QueueType::Fifo)
     , queueLength(0)
     , processorCount(0)
   {}
};

struct Station : public StationParams
{
    int id;

    Station()
     : id(0)
    {}

    void setParams(const StationParams& params)
    {
        *(static_cast<StationParams*>(this)) = params;
    }
};

