#ifndef WALLETDIALOG_H
#define WALLETDIALOG_H

#include <QDialog>
#include "clientnetworkmanager.h"
#include "rechargedialog.h"

// 网络消息结构体前向声明
struct NetworkMessage;

namespace Ui {
class WalletDialog;
}

class WalletDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WalletDialog(const QString &username,
                          ClientNetworkManager *networkManager,
                          double initialBalance,  // 添加初始余额参数
                          QWidget *parent = nullptr);
    ~WalletDialog();

signals:
    void balanceUpdated();  // 余额更新信号

public slots:
    // 添加公共消息处理方法，允许MainWindow转发消息
    void onMessageReceived(const NetworkMessage &message);

    // 添加设置余额的方法
    void setBalance(double balance) {
        currentBalance = balance;
        updateBalanceDisplay(balance);
    }

private slots:
    void showRechargeDialog();  // 显示充值对话框
    void loadRechargeRecords();  // 加载充值记录
    void refreshWalletInfo();    // 手动刷新钱包信息（可选）

private:

    void updateBalanceDisplay(double balance);  // 更新余额显示
    void updateRechargeRecordsDisplay(const QJsonArray &records);  // 更新充值记录显示
    bool validateRechargeInput(double amount);  // 验证充值输入
    void processRechargeRequest(double amount);  // 处理充值请求

private:
    Ui::WalletDialog *ui;
    QString currentUsername;
    ClientNetworkManager *networkManager;
    double currentBalance;
};

#endif // WALLETDIALOG_H
