#include "ui/statistics_window.hpp"

#include "ui_statistics_window.h"

#include <qwt_plot_curve.h>

#include <QDebug>

#include <cmath>


static const int points = 500;

StatisticsWindow::StatisticsWindow()
 : m_ui(new Ui::StatisticsWindow())
{
    m_ui->setupUi(this);

    m_ui->plotWidget->setTitle(QString::fromUtf8("Przykładowy wykres"));

    m_ui->plotWidget->setAxisTitle(QwtPlot::xBottom, "x");
    m_ui->plotWidget->setAxisTitle(QwtPlot::yLeft, "y");

    m_ui->plotWidget->setAxisScale(QwtPlot::yLeft, -2.0, 2.0);
    m_ui->plotWidget->setAxisScale(QwtPlot::xBottom, 0.0, 10.0);

    for (int i = 0; i < points; i++)
    {
        qreal x = (3.0 * M_PI / points) * i;

        m_curve1Samples.append(QPointF(x, 2.0 * std::sin(x)));
        m_curve2Samples.append(QPointF(x, -std::cos(x)));
    }

    m_curve1 = new QwtPlotCurve("Curve 1");
    m_curve2 = new QwtPlotCurve("Curve 2");

    m_curve1->setPen(Qt::red, 2.0);
    m_curve2->setPen(Qt::blue);

    m_curve1->setSamples(m_curve1Samples);
    m_curve2->setSamples(m_curve2Samples);

    m_curve1->attach(m_ui->plotWidget);
    m_curve2->attach(m_ui->plotWidget);

    m_ui->plotWidget->replot();

    connect(m_ui->testButton, SIGNAL(clicked()),
            this, SLOT(testButtonClicked()));

    m_animationTimer.setInterval(10);
    connect(&m_animationTimer, SIGNAL(timeout()),
            this, SLOT(animationStep()));
}

StatisticsWindow::~StatisticsWindow()
{
    delete m_ui;
    m_ui = nullptr;
}

void StatisticsWindow::testButtonClicked()
{
    if (!m_animationTimer.isActive())
    {
        m_curve1Samples.clear();
        m_curve2Samples.clear();

        m_curve1->setSamples(m_curve1Samples);
        m_curve2->setSamples(m_curve2Samples);

        m_animationTimer.start();
    }
}

void StatisticsWindow::animationStep()
{
    if (m_curve1Samples.size() >= points)
    {
        m_animationTimer.stop();
        return;
    }

    qreal x = (3.0 * M_PI / points) * m_curve1Samples.size();

    m_curve1Samples.append(QPointF(x, 2.0 * std::sin(x)));
    m_curve2Samples.append(QPointF(x, -std::cos(x)));

    m_curve1->setSamples(m_curve1Samples);
    m_curve2->setSamples(m_curve2Samples);

    m_ui->plotWidget->replot();
}

void StatisticsWindow::newEvent(Event event)
{
    qDebug() << "New event at statistics window!";
}
