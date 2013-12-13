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

    connectControls();

    setStackedWidgetIndex(CONSTANT_PAGE_INDEX);
}

DistributionParamsWidget::~DistributionParamsWidget()
{
    delete m_ui;
    m_ui = nullptr;
}

void DistributionParamsWidget::connectControls()
{
    connect(m_ui->distributionComboBox, SIGNAL(currentIndexChanged(int)),
            this, SIGNAL(distributionParamsChanged()));

    connect(m_ui->constantValueSpinBox, SIGNAL(valueChanged(double)),
            this, SIGNAL(distributionParamsChanged()));

    connect(m_ui->uniformMinSpinBox, SIGNAL(valueChanged(double)),
            this, SIGNAL(distributionParamsChanged()));
    connect(m_ui->uniformMaxSpinBox, SIGNAL(valueChanged(double)),
            this, SIGNAL(distributionParamsChanged()));

    connect(m_ui->normalMeanSpinBox, SIGNAL(valueChanged(double)),
            this, SIGNAL(distributionParamsChanged()));
    connect(m_ui->normalStdDevSpinBox, SIGNAL(valueChanged(double)),
            this, SIGNAL(distributionParamsChanged()));

    connect(m_ui->exponentialMeanSpinBox, SIGNAL(valueChanged(double)),
            this, SIGNAL(distributionParamsChanged()));
}

void DistributionParamsWidget::disconnectControls()
{
    disconnect(m_ui->distributionComboBox, SIGNAL(currentIndexChanged(int)),
               this, SIGNAL(distributionParamsChanged()));

    disconnect(m_ui->constantValueSpinBox, SIGNAL(valueChanged(double)),
               this, SIGNAL(distributionParamsChanged()));

    disconnect(m_ui->uniformMinSpinBox, SIGNAL(valueChanged(double)),
               this, SIGNAL(distributionParamsChanged()));
    disconnect(m_ui->uniformMaxSpinBox, SIGNAL(valueChanged(double)),
               this, SIGNAL(distributionParamsChanged()));

    disconnect(m_ui->normalMeanSpinBox, SIGNAL(valueChanged(double)),
               this, SIGNAL(distributionParamsChanged()));
    disconnect(m_ui->normalStdDevSpinBox, SIGNAL(valueChanged(double)),
               this, SIGNAL(distributionParamsChanged()));

    disconnect(m_ui->exponentialMeanSpinBox, SIGNAL(valueChanged(double)),
               this, SIGNAL(distributionParamsChanged()));
}

Distribution DistributionParamsWidget::getDistributionParams()
{
    Distribution distributionParams;

    switch (m_ui->distributionComboBox->currentIndex())
    {
        case CONSTANT_PAGE_INDEX:
            distributionParams.type = DistributionType::Uniform;
            distributionParams.param1 = m_ui->constantValueSpinBox->value();
            break;

        case UNIFORM_PAGE_INDEX:
            distributionParams.type = DistributionType::Uniform;
            distributionParams.param1 = m_ui->uniformMinSpinBox->value();
            distributionParams.param2 = m_ui->uniformMaxSpinBox->value();
            break;

        case NORMAL_PAGE_INDEX:
            distributionParams.type = DistributionType::Normal;
            distributionParams.param1 = m_ui->normalMeanSpinBox->value();
            distributionParams.param2 = m_ui->normalStdDevSpinBox->value();
            break;

        case EXPONENTIAL_PAGE_INDEX:
            distributionParams.type = DistributionType::Exponential;
            distributionParams.param1 = m_ui->exponentialMeanSpinBox->value();
            break;

        default:
            break;
    }

    return distributionParams;
}

void DistributionParamsWidget::setDistributionParams(const Distribution& distributionParams)
{
    disconnectControls();

    switch (distributionParams.type)
    {
        case DistributionType::Constant:
            m_ui->distributionComboBox->setCurrentIndex(CONSTANT_PAGE_INDEX);
            m_ui->constantValueSpinBox->setValue(distributionParams.param1);
            break;

        case DistributionType::Uniform:
            m_ui->distributionComboBox->setCurrentIndex(UNIFORM_PAGE_INDEX);
            m_ui->uniformMinSpinBox->setValue(distributionParams.param1);
            m_ui->uniformMaxSpinBox->setValue(distributionParams.param2);
            break;

        case DistributionType::Normal:
            m_ui->distributionComboBox->setCurrentIndex(NORMAL_PAGE_INDEX);
            m_ui->normalMeanSpinBox->setValue(distributionParams.param1);
            m_ui->normalStdDevSpinBox->setValue(distributionParams.param2);
            break;

        case DistributionType::Exponential:
            m_ui->distributionComboBox->setCurrentIndex(EXPONENTIAL_PAGE_INDEX);
            m_ui->exponentialMeanSpinBox->setValue(distributionParams.param1);
            break;

        default:
            break;
    }

    connectControls();
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
