#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QButtonGroup>
#include <QDate>
#include <QListWidgetItem>  // 添加这行，修复 QListWidgetItem 未声明错误
#include <QPushButton>      // 添加这行，修复 QPushButton 未声明错误

#include "../Common/flight.h"
#include "clientnetworkmanager.h"

// 前向声明
namespace Ui {
class MainWindow;
}

// 航班列表项Widget声明
class FlightItemWidget;

// 网络消息结构体前向声明
struct NetworkMessage;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(const QString &username, ClientNetworkManager* networkManager, QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void logoutRequested();  // 登出信号

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onSearchButtonClicked();
    void onFlightItemDoubleClicked(QListWidgetItem *item);
    void onLogoutButtonClicked();
    void onSwapButtonClicked();
    void onAirlineFilterChanged(int index);

    // 日期选择相关
    void onPrevWeekClicked();
    void onNextWeekClicked();
    void onDateButtonClicked();
    void onCalendarButtonClicked();

    // 导航相关
    void onBookingNavButtonClicked();
    void onMyNavButtonClicked();

    // 用户管理相关
    void onModifyUsernameClicked();
    void onModifyPasswordClicked();
    void onRechargeButtonClicked();
    void onViewAllOrdersButtonClicked();

    // 消息处理
    void onMessageReceived(const NetworkMessage &message);

    // 用户信息更新
    void onUsernameChanged(const QString& newUsername);

    // 订单管理
    void onCancelOrderClicked(int orderId, const QString &bookingNumber);

private:
    void setupConnections();
    void setupDateSelection();
    void setupNavigation();

    void updateDateButtons();
    void updateNavButtonStyles();

    void searchFlights();
    void searchFlightsByDate(const QDate &date);
    void displayFlights(const QList<Flight> &flights);
    void addFlightItem(const Flight &flight);

    void showFlightDetail(const Flight &flight);
    void showCalendarDialog();

    // 用户信息管理
    void loadUserInfo();
    void updateUserDisplay(const QString &username);

    // 钱包管理
    void queryWalletBalance();  // 查询钱包余额
    void updateWalletDisplay(); // 更新钱包显示

    // 订单管理
    void loadOrders();
    void displayOrders(const QJsonArray &orders);

private:
    Ui::MainWindow *ui;

    QString currentUsername;
    ClientNetworkManager* networkManager;

    // 钱包相关
    double userWalletBalance;
    bool walletBalanceChecked;

    // 航班数据
    QList<Flight> currentFlights;

    // 日期选择相关
    QButtonGroup *dateButtonGroup;
    QList<QPushButton*> dateButtons;  // 现在 QPushButton 已声明
    QDate selectedDate;
    QDate currentStartDate;

    // 导航相关
    QButtonGroup *navButtonGroup;

    // UI控件指针（如需要）
};

#endif // MAINWINDOW_H
