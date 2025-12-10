#ifndef BOOKINGDIALOG_H
#define BOOKINGDIALOG_H

#include <QDialog>
#include <QTimer>
#include "../Common/flight.h"
#include "../Common/cabin.h"
#include "clientnetworkmanager.h"

namespace Ui {
class BookingDialog;
}

class BookingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BookingDialog(const Flight &flight, const Cabin &cabin,
                           const QString &username,
                           ClientNetworkManager *networkManager,
                           QWidget *parent = nullptr);
    ~BookingDialog();

signals:
    void bookingSuccess();  // 预订成功信号

private slots:
    void onBookButtonClicked();
    void onCancelButtonClicked();
    void onMessageReceived(const NetworkMessage &message);
    void onRechargeButtonClicked();  // 充值按钮
    void checkInputValidity();       // 检查输入有效性

private:
    bool validateInput();
    void showSuccessMessage(const QString &bookingNumber);
    void queryWalletBalance();       // 查询钱包余额
    void processBooking();           // 处理预订
    void updateBalanceDisplay();     // 更新余额显示
    void updateUIState();            // 更新UI状态

private:
    Ui::BookingDialog *ui;
    Flight flight;
    Cabin cabin;
    QString username;
    ClientNetworkManager *networkManager;

    // 新增：余额相关变量
    double userBalance;
    bool balanceChecked;
    bool isBalanceSufficient;
    QTimer *inputCheckTimer;
};

#endif // BOOKINGDIALOG_H
