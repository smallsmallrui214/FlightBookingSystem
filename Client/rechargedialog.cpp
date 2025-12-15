#include "rechargedialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QDoubleValidator>
#include <QMessageBox>

RechargeDialog::RechargeDialog(double currentBalance, QWidget *parent)
    : QDialog(parent), currentBalance(currentBalance)
{
    setWindowTitle("钱包充值");
    setFixedSize(400, 200);
    setModal(true);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 创建表单布局
    QFormLayout *formLayout = new QFormLayout();

    // 当前余额显示
    QLabel *balanceLabel = new QLabel(QString("当前余额: ¥%1").arg(currentBalance, 0, 'f', 2));
    balanceLabel->setStyleSheet("font-size: 14px; color: #333; font-weight: bold;");
    formLayout->addRow("", balanceLabel);

    // 充值金额输入
    QLabel *amountLabel = new QLabel("充值金额:");
    amountEdit = new QLineEdit();
    amountEdit->setPlaceholderText("请输入充值金额");
    amountEdit->setValidator(new QDoubleValidator(1.0, 10000.0, 2, this));
    formLayout->addRow(amountLabel, amountEdit);

    // 充值后余额显示
    afterBalanceLabel = new QLabel("充值后余额: -");
    afterBalanceLabel->setStyleSheet("font-size: 13px; color: #666;");
    formLayout->addRow("", afterBalanceLabel);

    mainLayout->addLayout(formLayout);

    // 按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *cancelButton = new QPushButton("关闭");
    confirmButton = new QPushButton("确认充值");

    // 样式设置
    cancelButton->setStyleSheet(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #1E90FF, stop:1 #4169E1);"
        "    color: white;"
        "    border: none;"
        "    border-radius: 6px;"
        "    padding: 8px 20px;"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #4169E1, stop:1 #0000CD);"
        "}"
        );

    confirmButton->setStyleSheet(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #28a745, stop:1 #20c997);"
        "    color: white;"
        "    border: none;"
        "    border-radius: 6px;"
        "    padding: 8px 20px;"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #20c997, stop:1 #17a2b8);"
        "}"
        );

    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(confirmButton);
    mainLayout->addLayout(buttonLayout);

    // 连接信号
    connect(amountEdit, &QLineEdit::textChanged, this, &RechargeDialog::updateAfterBalance);
    connect(cancelButton, &QPushButton::clicked, this, &RechargeDialog::reject);
    connect(confirmButton, &QPushButton::clicked, this, &RechargeDialog::onConfirmClicked);
}

double RechargeDialog::getAmount() const {
    return amountEdit->text().toDouble();
}

void RechargeDialog::updateAfterBalance() {
    double amount = amountEdit->text().toDouble();
    double afterBalance = currentBalance + amount;
    afterBalanceLabel->setText(QString("充值后余额: ¥%1").arg(afterBalance, 0, 'f', 2));
}

void RechargeDialog::onConfirmClicked() {
    double amount = getAmount();
    if (amount <= 0) {
        QMessageBox::warning(this, "输入错误", "充值金额必须大于0");
        return;
    }
    if (amount > 10000) {
        QMessageBox::warning(this, "输入错误", "单次充值金额不能超过10000元");
        return;
    }
    emit rechargeConfirmed(amount);
    accept();
}
