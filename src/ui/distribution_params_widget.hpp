#pragma once

#include <QFrame>

namespace Ui
{
    class DistributionParamsWidget;
}

class DistributionParamsWidget : public QFrame
{
    Q_OBJECT
public:
    explicit DistributionParamsWidget(QWidget* parent);
    virtual ~DistributionParamsWidget();

private slots:
    void setStackedWidgetIndex(int index);

private:
    Ui::DistributionParamsWidget* m_ui;
};