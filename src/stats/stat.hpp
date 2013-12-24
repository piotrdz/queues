#pragma once

#include "engine/event.hpp"

class Stat
{
public:
    Stat() :
     m_value(0.0)
    {}
    virtual ~Stat() {}

    virtual bool update(Event event) = 0;
    virtual void reset() = 0;

    double getValue() const
    {
        return m_value;
    }

protected:
    double m_value;
};
