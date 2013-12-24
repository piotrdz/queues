#include "ui/statistics_series_data.hpp"

#include "stats/stat.hpp"


StatisticsSeriesData::StatisticsSeriesData(Stat* stat)
 : m_stat(stat)
 , m_minSample(0.0)
 , m_maxSample(0.0)
{}

StatisticsSeriesData::~StatisticsSeriesData()
{
    delete m_stat;
    m_stat = nullptr;
}

bool StatisticsSeriesData::update(Event event)
{
    if (m_stat->update(event))
    {
        double value = m_stat->getValue();
        if (m_samples.empty())
        {
            m_minSample = m_maxSample = value;
        }
        else
        {
            if (value > m_maxSample)
            {
                m_maxSample = value;
            }

            if (value < m_minSample)
            {
                m_minSample = value;
            }
        }
        m_samples.append(QPointF(event.time, value));

        return true;
    }

    return false;
}

void StatisticsSeriesData::reset()
{
    m_stat->reset();
    m_samples.clear();
}

size_t StatisticsSeriesData::size() const
{
    return m_samples.size();
}

QPointF StatisticsSeriesData::sample(size_t i) const
{
    return m_samples.at(i);
}

QRectF StatisticsSeriesData::boundingRect() const
{
    if (m_samples.empty())
    {
        return QRectF(QPointF(0.0, 1.0), QSizeF(1.0, 1.0));
    }

    double maxTime = m_samples.back().x();
    QRectF rect(QPointF(0.0, m_minSample), QSizeF(maxTime, m_maxSample - m_minSample));
    return rect;
}
