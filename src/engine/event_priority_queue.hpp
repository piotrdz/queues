#pragma once

#include "engine/event.hpp"

#include <QQueue>


class EventPriorityQueue
{
public:
    void enqueue(Event event);
    Event dequeue();
    Event head();

    bool isEmpty() const;
    void clear();

    QList<Event> getAll() const;

private:
    QQueue<Event> m_queue;
};
