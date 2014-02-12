#include "ui/statistic_item_widget.hpp"

#include "ui_statistic_item_widget.h"


StatisticItemWidget::StatisticItemWidget(QWidget* parent, int index)
 : QWidget(parent)
 , m_index(0)
 , m_ui(new Ui::StatisticItemWidget())
{
    m_ui->setupUi(this);

    setIndex(index);

    m_ui->typeComboBox->addItem(QString::fromUtf8("T_t syst."),
                                static_cast<int>(StatType::SystemMeanTaskProcessingTime));
    m_ui->typeComboBox->addItem(QString::fromUtf8("N_t syst."),
                                static_cast<int>(StatType::SystemMeanNumberOfTasks));
    m_ui->typeComboBox->addItem(QString::fromUtf8("U_p stat."),
                                static_cast<int>(StatType::StationMeanUtilizedProcessors));
    m_ui->typeComboBox->addItem(QString::fromUtf8("T_q stat."),
                                static_cast<int>(StatType::StationMeanWaitTime));
    m_ui->typeComboBox->addItem(QString::fromUtf8("N_q stat."),
                                static_cast<int>(StatType::StationMeanQueueLength));

    m_ui->stationLineEdit->setValidator(new QIntValidator(1, 100));

    adjustOnTypeChanged();

    connect(m_ui->typeComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(typeChanged()));

    connect(m_ui->stationLineEdit, SIGNAL(textEdited(QString)),
            this, SLOT(typeChanged()));

    connect(m_ui->removeButton, SIGNAL(clicked()),
            this, SLOT(removeButtonClicked()));
}

StatisticItemWidget::~StatisticItemWidget()
{
    delete m_ui;
    m_ui = nullptr;
}

void StatisticItemWidget::setIndex(int index)
{
    m_index = index;
    m_ui->numberLabel->setText(QString("#%1").arg(index+1));
}

int StatisticItemWidget::getIndex() const
{
    return m_index;
}

StatType StatisticItemWidget::getType() const
{
    StatType type = static_cast<StatType>(m_ui->typeComboBox->currentData().toInt());
    return type;
}

int StatisticItemWidget::getStationId() const
{
    int stationId = m_ui->stationLineEdit->text().toInt();
    return stationId;
}

void StatisticItemWidget::adjustOnTypeChanged()
{
    StatType statType = getType();
    if (statType == StatType::SystemMeanNumberOfTasks ||
        statType == StatType::SystemMeanTaskProcessingTime)
    {
        m_ui->stationLabel->setVisible(false);
        m_ui->stationLineEdit->setVisible(false);
    }
    else
    {
        m_ui->stationLabel->setVisible(true);
        m_ui->stationLineEdit->setVisible(true);
    }
}

void StatisticItemWidget::typeChanged()
{
    adjustOnTypeChanged();
    emit typeChanged(m_index);
}

void StatisticItemWidget::removeButtonClicked()
{
    emit removeRequested(m_index);
}
