#pragma once

#include <QTimer>
#include <QWidget>

namespace Ui
{
    class StatisticsWindow;
}

class QwtPlotCurve;

class StatisticsWindow : public QWidget
{
Q_OBJECT
public:
    StatisticsWindow();
    virtual ~StatisticsWindow();

private slots:
    void testButtonClicked();
    void animationStep();

private:
    Ui::StatisticsWindow* m_ui;
    QTimer m_animationTimer;
    QwtPlotCurve* m_curve1;
    QwtPlotCurve* m_curve2;
    QVector<QPointF> m_curve1Samples;
    QVector<QPointF> m_curve2Samples;
};
