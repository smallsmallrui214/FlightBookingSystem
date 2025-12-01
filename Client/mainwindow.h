#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QTimer>           // 添加QTimer头文件
#include <QJsonArray>       // 添加QJsonArray头文件
#include <QDateTime>        // 添加QDateTime头文件
#include <QDate>            // 添加QDate头文件
#include <QButtonGroup>     // 添加QButtonGroup头文件
#include <QPushButton>      // 添加QPushButton头文件
#include "clientnetworkmanager.h"
#include "../Common/flight.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(const QString &username, ClientNetworkManager* networkManager = nullptr, QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void logoutRequested();  // 添加登出信号

private slots:
    void onSearchButtonClicked();
    void onSortChanged(int index);
    void onFlightItemDoubleClicked(QListWidgetItem *item);
    void onMessageReceived(const NetworkMessage &message);
    void onLogoutButtonClicked();
    void onSwapButtonClicked();
    void onAirlineFilterChanged(int index);
    // 新增日期选择槽函数
    void onDateButtonClicked();
    void onPrevWeekClicked();
    void onNextWeekClicked();
    void onCalendarButtonClicked();

private:
    void setupConnections();
    void searchFlights();
    void displayFlights(const QList<Flight> &flights);
    void addFlightItem(const Flight &flight);
    void showFlightDetail(const Flight &flight);
    // 新增日期选择相关方法
    void setupDateSelection();
    void updateDateButtons();
    void searchFlightsByDate(const QDate &date);
    void showCalendarDialog();

    Ui::MainWindow *ui;
    QString currentUsername;
    ClientNetworkManager* networkManager;
    QList<Flight> currentFlights;
    // 新增日期选择相关成员变量
    QDate currentCenterDate;  // 当前显示的日期范围中心
    QDate selectedDate;       // 当前选中的日期
    QButtonGroup *dateButtonGroup;
    QList<QPushButton*> dateButtons;
     QDate currentStartDate;  // 当前显示的起始日期
};

#endif // MAINWINDOW_H
