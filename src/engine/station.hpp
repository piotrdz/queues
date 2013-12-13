#pragma once

#include "engine/distribution.hpp"

#include <QPointF>

enum class QueueType
{
    Fifo,
    Random
};

struct Station
{
    int id;
    QueueType queueType;
    int queueLength;
    int processorCount;
    Distribution serviceTimeDistribution;
    QPointF position;

    Station()
     : id(0)
     , queueType(QueueType::Fifo)
     , queueLength(0)
     , processorCount(0)
   {}
};

