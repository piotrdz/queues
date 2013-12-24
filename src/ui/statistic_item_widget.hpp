#pragma once

#include <QWidget>

namespace Ui
{
    class StatisticItemWidget;
}

class StatisticItemWidget : public QWidget
{
public:
    explicit StatisticItemWidget(QWidget* parent);
    virtual ~StatisticItemWidget();

private:
    Ui::StatisticItemWidget* m_ui;
};
