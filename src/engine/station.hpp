#pragma once

enum class QueueType
{
    Fifo,
    Random
};

enum class ServiceDistribution
{
    Constant,
    Uniform,
    Normal,
    Exponential
};

struct Station
{
    int id;
    QueueType queueType;
    int queueLength;
    int processorCount;
    ServiceDistribution serviceDistribution;
    double serviceDistributionParam1, serviceDistributionParam2;

    Station()
     : id(0)
     , queueType(QueueType::Fifo)
     , queueLength(0)
     , processorCount(0)
     , serviceDistribution(ServiceDistribution::Constant)
     , serviceDistributionParam1(0.0)
     , serviceDistributionParam2(0.0)
   {}
};

