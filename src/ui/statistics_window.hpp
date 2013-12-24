#pragma once

#include "engine/event.hpp"

#include <QList>
#include <QTimer>
#include <QWidget>

namespace Ui
{
    class StatisticsWindow;
}

class QwtPlotCurve;
class StatisticsSeriesData;

class StatisticsWindow : public QWidget
{
Q_OBJECT
public:
    StatisticsWindow();
    virtual ~StatisticsWindow();

    void reset();

public slots:
    void newEvent(Event event);

private slots:
    void addNewStatistic();

private:
    Ui::StatisticsWindow* m_ui;
    struct StatisticsData
    {
        QwtPlotCurve* curve;
        StatisticsSeriesData* seriesData;
    };
    QList<StatisticsData> m_statisticsData;
};
