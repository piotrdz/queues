#pragma once

#include "engine/station.hpp"

struct Connection
{
    int from;
    int to;
    int weight;

    Connection()
     : from(INVALID_STATION_ID)
     , to(INVALID_STATION_ID)
     , weight(0)
    {}
};

