#include "engine/event_priority_queue.hpp"

void EventPriorityQueue::enqueue(Event event)
{
    if (m_queue.isEmpty() || event.time > m_queue.back().time)
    {
        m_queue.enqueue(event);
    }
    else
    {
        auto it = m_queue.begin();
        for (; it != m_queue.end(); ++it)
        {
            if (event.time < it->time)
            {
                break;
            }
        }

        m_queue.insert(it, event);
    }
}

Event EventPriorityQueue::dequeue()
{
    return m_queue.dequeue();
}

Event EventPriorityQueue::head()
{
    return m_queue.head();
}

bool EventPriorityQueue::isEmpty() const
{
    return m_queue.isEmpty();
}

void EventPriorityQueue::clear()
{
    m_queue.clear();
}

QList<Event> EventPriorityQueue::getAll() const
{
    return m_queue;
}

