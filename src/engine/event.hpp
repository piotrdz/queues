#pragma once

#include "engine/station.hpp"

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
     , taskId(0)
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
