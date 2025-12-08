#ifndef WALLETDIALOG_H
#define WALLETDIALOG_H

#include <QDialog>
#include "clientnetworkmanager.h"

namespace Ui {
class WalletDialog;
}

class WalletDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WalletDialog(const QString &username,
                          ClientNetworkManager *networkManager,
                          QWidget *parent = nullptr);
    ~WalletDialog();

private slots:
    void onRechargeButtonClicked();
    void onCloseButtonClicked();
    void onMessageReceived(const NetworkMessage &message);
    void onRechargeAmountChanged(const QString &text);

private:
    void loadWalletInfo();
    void updateBalanceDisplay(double balance);
    bool validateRechargeInput();

private:
    Ui::WalletDialog *ui;
    QString currentUsername;
    ClientNetworkManager *networkManager;
    double currentBalance;
};

#endif // WALLETDIALOG_H
