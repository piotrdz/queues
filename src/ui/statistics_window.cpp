#include "ui/statistics_window.hpp"

#include "stats/station_stats.hpp"
#include "stats/system_stats.hpp"

#include "ui/statistics_series_data.hpp"
#include "statistic_item_widget.hpp"

#include "ui_statistics_window.h"

#include <qwt_plot_curve.h>

#include <QMessageBox>

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
 , m_statisticsLayout(nullptr)
{
    m_ui->setupUi(this);

    m_statisticsLayout = new QVBoxLayout(m_ui->statisticsScrollArea);
    m_statisticsLayout->addSpacerItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

    m_ui->plotWidget->setAxisTitle(QwtPlot::xBottom, "czas [s]");

    addNewStatistic();

    m_ui->plotWidget->replot();

    connect(m_ui->addButton, SIGNAL(clicked()),
            this, SLOT(addNewStatistic()));
    connect(m_ui->infoButton, SIGNAL(clicked()),
            this, SLOT(showStatisticInfo()));
}

StatisticsWindow::~StatisticsWindow()
{
    delete m_ui;
    m_ui = nullptr;

    m_statisticsLayout = nullptr;

    m_statisticsData.clear();
}

void StatisticsWindow::addNewStatistic()
{
    int statisticIndex = m_statisticsData.size();

    StatisticsData statisticsData;

    statisticsData.seriesData = new StatisticsSeriesData(new SystemMeanTaskProcessingTimeStat());

    statisticsData.curve = new QwtPlotCurve();
    statisticsData.curve->setPen(QPen(PEN_COLORS.at(statisticIndex % PEN_COLORS.size())));
    statisticsData.curve->setData(statisticsData.seriesData);
    statisticsData.curve->attach(m_ui->plotWidget);

    statisticsData.widget = new StatisticItemWidget(m_ui->statisticsScrollArea, statisticIndex);
    m_statisticsLayout->insertWidget(statisticIndex, statisticsData.widget);

    connect(statisticsData.widget, SIGNAL(typeChanged(int)),
            this, SLOT(statisticTypeChanged(int)));
    connect(statisticsData.widget, SIGNAL(removeRequested(int)),
            this, SLOT(removeStatistic(int)));

    m_statisticsData.append(statisticsData);

    m_ui->plotWidget->replot();
}

void StatisticsWindow::showStatisticInfo()
{
    QMessageBox::information(this, QString::fromUtf8("Kolejki"),
            QString::fromUtf8(
                "Dostępne statystyki:\n"
                "  T_t syst. - średni czas przetwarzania zadania w całym systemie\n"
                "  N_t syst. - średnia liczba zadań w całym systemie\n"
                "  U_p stac. - średnia liczba procesorów w użyciu w danej stacji\n"
                "  T_q stac. - średni czas oczekiwania na obsługę w danej stacji\n"
                "  N_q stac. - średnia długość kolejki w danej stacji\n"
            ));
}

void StatisticsWindow::statisticTypeChanged(int index)
{
    Q_ASSERT(index >= 0 && index < m_statisticsData.size());

    StatisticsData& data = m_statisticsData[index];

    StatType statType = data.widget->getType();
    int stationId = data.widget->getStationId();

    Stat* newStat = nullptr;
    switch (statType)
    {
        case StatType::SystemMeanTaskProcessingTime:
            newStat = new SystemMeanTaskProcessingTimeStat();
            break;

        case StatType::SystemMeanNumberOfTasks:
            newStat = new SystemMeanNumberOfTasksStat();
            break;

        case StatType::StationMeanUtilizedProcessors:
            newStat = new StationMeanUtilizedProcessorsStat(stationId);
            break;

        case StatType::StationMeanQueueLength:
            newStat = new StationMeanQueueLengthStat(stationId);
            break;

        case StatType::StationMeanWaitTime:
            newStat = new StationMeanWaitTimeStat(stationId);
            break;
    }

    data.curve->setData(nullptr);
    data.seriesData = new StatisticsSeriesData(newStat);
    data.curve->setData(data.seriesData);
}

void StatisticsWindow::removeStatistic(int index)
{
    Q_ASSERT(index >= 0 && index < m_statisticsData.size());

    {
        StatisticsData& data = m_statisticsData[index];

        m_ui->statisticsScrollArea->layout()->removeWidget(data.widget);
        delete data.widget;

        data.curve->detach();
        delete data.curve;
    }

    m_statisticsData.removeAt(index);

    int i = 0;
    for (StatisticsData& data : m_statisticsData)
    {
        data.widget->setIndex(i++);
    }
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
