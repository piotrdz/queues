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

const int INVALID_STATION_ID = 0;

const int INPUT_STATION_ID = -1;
const int OUTPUT_STATION_ID = -2;


struct Station : public StationParams
{
    int id;

    Station()
     : id(INVALID_STATION_ID)
    {}

    void setParams(const StationParams& params)
    {
        *(static_cast<StationParams*>(this)) = params;
    }
};

