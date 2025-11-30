#include "calendardialog.h"
#include <QCalendarWidget>
#include <QDialogButtonBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDebug>

CalendarDialog::CalendarDialog(QWidget *parent)
    : QDialog(parent), selectedDate(QDate::currentDate())
{
    setupUI();
}

void CalendarDialog::setupUI()
{
    setWindowTitle("选择出行日期");
    setMinimumSize(450, 400);
    setModal(true);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 月份导航和标题
    QHBoxLayout *headerLayout = new QHBoxLayout();

    monthLabel = new QLabel();
    monthLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #1e88e5; padding: 10px;");
    monthLabel->setAlignment(Qt::AlignCenter);

    headerLayout->addWidget(monthLabel);
    mainLayout->addLayout(headerLayout);

    // 日历控件
    calendarWidget = new QCalendarWidget(this);
    calendarWidget->setGridVisible(true);
    calendarWidget->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);

    // 设置日历样式 - 增强版，更好地区分过去日期
    calendarWidget->setStyleSheet(
        "QCalendarWidget {"
        "    background: white;"
        "    border: 2px solid #e3f2fd;"
        "    border-radius: 8px;"
        "}"
        "QCalendarWidget QToolButton {"
        "    background-color: #1e88e5;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 4px;"
        "    padding: 5px 10px;"
        "    font-weight: bold;"
        "    min-height: 25px;"
        "}"
        "QCalendarWidget QMenu {"
        "    background: white;"
        "    border: 1px solid #ddd;"
        "}"
        "QCalendarWidget QSpinBox {"
        "    background: white;"
        "    border: 1px solid #ddd;"
        "    border-radius: 4px;"
        "    padding: 2px;"
        "}"
        "QCalendarWidget QWidget {"
        "    alternate-background-color: #f5f5f5;"
        "}"
        "QCalendarWidget QAbstractItemView:enabled {"
        "    color: #333;"
        "    background: white;"
        "    selection-background-color: #1e88e5;"
        "    selection-color: white;"
        "}"
        "QCalendarWidget QAbstractItemView:disabled {"
        "    color: #cccccc;"
        "    background: #f8f8f8;"
        "    text-decoration: line-through;"
        "}"
        "QCalendarWidget QTableView {"
        "    gridline-color: #e0e0e0;"
        "}"
        "QCalendarWidget QTableView::item:disabled {"
        "    background-color: #f0f0f0;"
        "    color: #aaaaaa;"
        "}"
        "QCalendarWidget QTableView::item:selected:disabled {"
        "    background-color: #e0e0e0;"
        "    color: #888888;"
        "}"
        );

    mainLayout->addWidget(calendarWidget);

    // 按钮
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    mainLayout->addWidget(buttonBox);

    // 连接信号
    connect(calendarWidget, &QCalendarWidget::clicked, this, &CalendarDialog::onDateSelected);
    connect(calendarWidget, &QCalendarWidget::currentPageChanged, this, &CalendarDialog::onMonthChanged);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // 初始设置
    calendarWidget->setSelectedDate(selectedDate);
    updateMonthLabel();
}

void CalendarDialog::setSelectedDate(const QDate &date)
{
    if (date.isValid()) {
        selectedDate = date;
        calendarWidget->setSelectedDate(date);
        updateMonthLabel();
    }
}

void CalendarDialog::setDateRange(const QDate &minDate, const QDate &maxDate)
{
    this->minDate = minDate;
    this->maxDate = maxDate;
    calendarWidget->setMinimumDate(minDate);
    calendarWidget->setMaximumDate(maxDate);
}

QDate CalendarDialog::getSelectedDate() const
{
    return selectedDate;
}

void CalendarDialog::onDateSelected(const QDate &date)
{
    selectedDate = date;
}

void CalendarDialog::onMonthChanged(int year, int month)
{
    Q_UNUSED(year)
    Q_UNUSED(month)
    updateMonthLabel();  // 月份变化时更新标签
}

void CalendarDialog::updateMonthLabel()
{
    // 获取当前显示的年份和月份
    int year = calendarWidget->yearShown();
    int month = calendarWidget->monthShown();

    QString monthText = QString("%1年%2月").arg(year).arg(month);
    monthLabel->setText(monthText);

    qDebug() << "月份标签更新为:" << monthText;
}
