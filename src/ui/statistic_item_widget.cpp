#include "ui/statistic_item_widget.hpp"

#include "ui_statistic_item_widget.h"


StatisticItemWidget::StatisticItemWidget(QWidget* parent)
 : QWidget(parent)
 , m_ui(new Ui::StatisticItemWidget())
{
    m_ui->setupUi(this);
}

StatisticItemWidget::~StatisticItemWidget()
{
    delete m_ui;
    m_ui = nullptr;
}

// TODO...
