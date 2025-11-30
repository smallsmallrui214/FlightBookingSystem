#ifndef CALENDARDIALOG_H
#define CALENDARDIALOG_H

#include <QDialog>
#include <QDate>

class QCalendarWidget;
class QDialogButtonBox;
class QLabel;

class CalendarDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CalendarDialog(QWidget *parent = nullptr);
    void setSelectedDate(const QDate &date);
    void setDateRange(const QDate &minDate, const QDate &maxDate);
    QDate getSelectedDate() const;

private slots:
    void onDateSelected(const QDate &date);
    void onMonthChanged(int year, int month);

private:
    void setupUI();
    void updateMonthLabel();

private:
    QCalendarWidget *calendarWidget;
    QDialogButtonBox *buttonBox;
    QLabel *monthLabel;
    QDate selectedDate;
    QDate minDate;
    QDate maxDate;
};

#endif // CALENDARDIALOG_H
