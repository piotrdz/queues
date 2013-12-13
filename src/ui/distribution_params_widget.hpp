#pragma once

#include "engine/distribution.hpp"

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

    Distribution getDistributionParams();
    void setDistributionParams(const Distribution& distributionParams);

signals:
    void distributionParamsChanged();

private slots:
    void setStackedWidgetIndex(int index);

private:
    void connectControls();
    void disconnectControls();

private:
    Ui::DistributionParamsWidget* m_ui;
};