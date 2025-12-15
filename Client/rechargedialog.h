#ifndef RECHARGEDIALOG_H
#define RECHARGEDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>

class RechargeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RechargeDialog(double currentBalance, QWidget *parent = nullptr);
    double getAmount() const;

signals:
    void rechargeConfirmed(double amount);

private slots:
    void updateAfterBalance();
    void onConfirmClicked();

private:
    double currentBalance;
    QLineEdit *amountEdit;
    QLabel *afterBalanceLabel;
    QPushButton *confirmButton;
};

#endif // RECHARGEDIALOG_H
