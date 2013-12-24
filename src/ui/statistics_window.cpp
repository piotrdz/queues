#include "ui/statistics_window.hpp"

#include "stats/station_stats.hpp"
#include "stats/system_stats.hpp"

#include "ui/statistics_series_data.hpp"

#include "ui_statistics_window.h"

#include <qwt_plot_curve.h>

#include <QDebug>

#include <cmath>

namespace
{
    static const QList<QColor> PEN_COLORS =
    {
        QColor("#FF0000"),
        QColor("#FFA858"),
        QColor("#C0C000"),
        QColor("#00C000"),
        QColor("#00C0C0"),
        QColor("#FF00FF"),
        QColor("#0000C0")
    };
}


StatisticsWindow::StatisticsWindow()
 : m_ui(new Ui::StatisticsWindow())
{
    m_ui->setupUi(this);

    m_ui->plotWidget->setAxisTitle(QwtPlot::xBottom, "czas [s]");

    addNewStatistic();

    m_ui->plotWidget->replot();
}

StatisticsWindow::~StatisticsWindow()
{
    delete m_ui;
    m_ui = nullptr;

    m_statisticsData.clear();
}

void StatisticsWindow::addNewStatistic()
{
    StatisticsData statisticsData;

    statisticsData.seriesData = new StatisticsSeriesData(new SystemMeanTaskProcessingTimeStat());

    statisticsData.curve = new QwtPlotCurve();
    statisticsData.curve->setPen(QPen(PEN_COLORS.at(m_statisticsData.size() % PEN_COLORS.size())));
    statisticsData.curve->setData(statisticsData.seriesData);
    statisticsData.curve->attach(m_ui->plotWidget);

    m_statisticsData.append(statisticsData);

    // TODO...

    m_ui->plotWidget->replot();
}

void StatisticsWindow::newEvent(Event event)
{
    bool update = false;
    for (StatisticsData& statisticsData : m_statisticsData)
    {
        bool updateSeries = statisticsData.seriesData->update(event);
        update = update || updateSeries;
    }

    if (update)
    {
        m_ui->plotWidget->replot();
    }
}

void StatisticsWindow::reset()
{
    for (StatisticsData& statisticsData : m_statisticsData)
    {
        statisticsData.seriesData->reset();
    }

    m_ui->plotWidget->replot();
}
