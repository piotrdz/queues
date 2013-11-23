#pragma once

struct Connection
{
    int from;
    int to;
    int weight;

    Connection()
     : from(0)
     , to(0)
     , weight(0)
    {}
};

