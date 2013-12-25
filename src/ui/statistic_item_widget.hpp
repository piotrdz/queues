#pragma once

#include "stats/stat_type.hpp"

#include <QWidget>

namespace Ui
{
    class StatisticItemWidget;
}

class StatisticItemWidget : public QWidget
{
    Q_OBJECT
public:
    explicit StatisticItemWidget(QWidget* parent, int index);
    virtual ~StatisticItemWidget();

    void setIndex(int index);
    int getIndex() const;
    StatType getType() const;
    int getStationId() const;

signals:
    void typeChanged(int index);
    void removeRequested(int index);

private slots:
    void typeChanged();
    void removeButtonClicked();

private:
    void adjustOnTypeChanged();

private:
    int m_index;
    Ui::StatisticItemWidget* m_ui;
};
