#include "ui/distribution_params_widget.hpp"

#include "ui_distribution_params_widget.h"

namespace
{
    const int CONSTANT_PAGE_INDEX = 0;
    const int UNIFORM_PAGE_INDEX = 1;
    const int NORMAL_PAGE_INDEX = 2;
    const int EXPONENTIAL_PAGE_INDEX = 3;
};


DistributionParamsWidget::DistributionParamsWidget(QWidget* parent)
 : QFrame(parent)
 , m_ui(new Ui::DistributionParamsWidget())
{
    m_ui->setupUi(this);

    connect(m_ui->distributionComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(setStackedWidgetIndex(int)));

    setStackedWidgetIndex(CONSTANT_PAGE_INDEX);
}

DistributionParamsWidget::~DistributionParamsWidget()
{
    delete m_ui;
    m_ui = nullptr;
}

void DistributionParamsWidget::setStackedWidgetIndex(int index)
{
    if (m_ui->paramsStackedWidget->currentWidget() != nullptr)
    {
        m_ui->paramsStackedWidget->currentWidget()->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    }

    m_ui->paramsStackedWidget->setCurrentIndex(index);
    m_ui->paramsStackedWidget->currentWidget()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
}

