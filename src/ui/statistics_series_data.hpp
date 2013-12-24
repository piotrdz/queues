#pragma once

#include "engine/event.hpp"

#include <qwt_series_data.h>

#include <QList>

class Stat;

class StatisticsSeriesData : public QwtSeriesData<QPointF>
{
public:
    StatisticsSeriesData(Stat* stat);
    virtual ~StatisticsSeriesData();

    bool update(Event event);
    void reset();

    virtual size_t size() const override;
    virtual QPointF sample(size_t i) const override;
    virtual QRectF boundingRect() const override;

private:
    Stat* m_stat;
    QList<QPointF> m_samples;
    double m_minSample, m_maxSample;
};
