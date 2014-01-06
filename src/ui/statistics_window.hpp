#pragma once

#include "engine/event.hpp"

#include <QList>
#include <QTimer>
#include <QWidget>

namespace Ui
{
    class StatisticsWindow;
}

class QVBoxLayout;
class QwtPlotCurve;
class StatisticItemWidget;
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
    void showStatisticInfo();
    void statisticTypeChanged(int index);
    void removeStatistic(int index);

private:
    Ui::StatisticsWindow* m_ui;
    struct StatisticsData
    {
        QwtPlotCurve* curve;
        StatisticsSeriesData* seriesData;
        StatisticItemWidget* widget;
    };
    QList<StatisticsData> m_statisticsData;
    QVBoxLayout* m_statisticsLayout;
};
