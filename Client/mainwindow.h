#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QTimer>
#include <QJsonArray>
#include <QDateTime>
#include <QDate>
#include <QButtonGroup>
#include <QPushButton>
#include "clientnetworkmanager.h"
#include "../Common/flight.h"
#include "walletdialog.h"  // 新增

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
    void logoutRequested();

private slots:
    void onSearchButtonClicked();
    void onFlightItemDoubleClicked(QListWidgetItem *item);
    void onMessageReceived(const NetworkMessage &message);
    void onLogoutButtonClicked();
    void onSwapButtonClicked();
    void onAirlineFilterChanged(int index);
    void onDateButtonClicked();
    void onPrevWeekClicked();
    void onNextWeekClicked();
    void onCalendarButtonClicked();
    // 新增的导航按钮槽函数
    void onBookingNavButtonClicked();
    void onMyNavButtonClicked();
    // 新增的其他槽函数
    void onRechargeButtonClicked();
    void onViewAllOrdersButtonClicked();

private:
    void setupConnections();
    void searchFlights();
    void displayFlights(const QList<Flight> &flights);
    void addFlightItem(const Flight &flight);
    void showFlightDetail(const Flight &flight);
    void setupDateSelection();
    void updateDateButtons();
    void searchFlightsByDate(const QDate &date);
    void showCalendarDialog();
    QString getSearchConditions() const;
    // 新增的导航相关函数
    void setupNavigation();
    void updateNavButtonStyles();
    // 新增的"我的"页面函数
    void loadUserInfo();
    void loadOrders();
    void displayOrders(const QJsonArray &orders);

    Ui::MainWindow *ui;
    QString currentUsername;
    ClientNetworkManager* networkManager;
    QList<Flight> currentFlights;
    QDate currentCenterDate;
    QDate selectedDate;
    QButtonGroup *dateButtonGroup;
    QList<QPushButton*> dateButtons;
    QDate currentStartDate;
    // 新增的导航按钮组
    QButtonGroup *navButtonGroup;
};

#endif // MAINWINDOW_H
