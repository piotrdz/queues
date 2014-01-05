#pragma once

#include "engine/station.hpp"

#include <QDebug>


enum class EventType
{
    TaskInput,             //!< wejście zdarzenia do sieci
    TaskOutput,            //!< wyjście zdarzenia z sieci
    TaskAddedToQueue,      //!< zdarzenie wchodzi do kolejki
    TaskStartedProcessing, //!< zdarzenie przetwarzane przez maszynę
    TaskEndedProcessing,   //!< zdarzenie zakonczylo przetwarzanie
    MachineIsIdle,         //!< maszyna została zwolniona
    QueueHasPlace          //!< miejsce w kolejce zostało zwolnione
};

const int EMPTY_TASK_ID = 0;

struct Event
{
    EventType type;
    double time;
    int stationId;
    int taskId;

    Event()
     : type(EventType::TaskInput)
     , time(0.0)
     , stationId(INVALID_STATION_ID)
     , taskId(EMPTY_TASK_ID)
    {}

    Event(EventType type,
          double time,
          int stationId,
          int taskId)
     : type(type)
     , time(time)
     , stationId(stationId)
     , taskId(taskId)
    {}
};

inline QDebug operator<<(QDebug dbg, EventType eventType)
{
    QString eventName;
    switch (eventType)
    {
        case EventType::TaskInput:
            eventName = "TaskInput";
            break;

        case EventType::TaskOutput:
            eventName = "TaskOutput";
            break;

        case EventType::TaskAddedToQueue:
            eventName = "TaskAddedToQueue";
            break;

        case EventType::TaskStartedProcessing:
            eventName = "TaskStartedProcessing";
            break;

        case EventType::TaskEndedProcessing:
            eventName = "TaskEndedProcessing";
            break;

        case EventType::MachineIsIdle:
            eventName = "MachineIsIdle";
            break;

        case EventType::QueueHasPlace:
            eventName = "QueueHasPlace";
            break;
    }

    dbg.space() << eventName;
    return dbg.space();
}
