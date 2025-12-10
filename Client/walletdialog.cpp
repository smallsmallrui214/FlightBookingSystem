#include "walletdialog.h"
#include "ui_walletdialog.h"
#include "../Common/protocol.h"
#include <QMessageBox>
#include <QDebug>
#include <QDoubleValidator>
#include <QDateTime>

WalletDialog::WalletDialog(const QString &username,
                           ClientNetworkManager *networkManager,
                           QWidget *parent)
    : QDialog(parent), ui(new Ui::WalletDialog),
    currentUsername(username), networkManager(networkManager),
    currentBalance(0.0)
{
    ui->setupUi(this);

    // 设置窗口标题
    setWindowTitle(QString("我的钱包 - %1").arg(username));

    // 设置充值金额输入验证
    ui->rechargeAmountEdit->setValidator(new QDoubleValidator(1.0, 10000.0, 2, this));
    ui->rechargeAmountEdit->setPlaceholderText("请输入充值金额");

    // 连接信号
    connect(ui->rechargeButton, &QPushButton::clicked, this, &WalletDialog::onRechargeButtonClicked);
    connect(ui->closeButton, &QPushButton::clicked, this, &WalletDialog::onCloseButtonClicked);
    connect(ui->rechargeAmountEdit, &QLineEdit::textChanged,
            this, &WalletDialog::onRechargeAmountChanged);
    connect(networkManager, &ClientNetworkManager::messageReceived,
            this, &WalletDialog::onMessageReceived);

    // 加载钱包信息
    loadWalletInfo();
}

WalletDialog::~WalletDialog()
{
    delete ui;
}

void WalletDialog::loadWalletInfo()
{
    if (!networkManager || !networkManager->isConnected()) {
        QMessageBox::warning(this, "错误", "未连接到服务器");
        return;
    }

    // 发送钱包查询请求
    NetworkMessage msg;
    msg.type = WALLET_QUERY_REQUEST;
    msg.data["username"] = currentUsername;

    networkManager->sendMessage(msg);
    qDebug() << "发送钱包查询请求，用户名:" << currentUsername;
}

void WalletDialog::updateBalanceDisplay(double balance)
{
    currentBalance = balance;

    // 更新余额显示
    QString balanceText = QString("¥%1").arg(balance, 0, 'f', 2);
    ui->balanceLabel->setText(balanceText);

    // 更新时间显示
    QString timeText = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    ui->lastUpdateLabel->setText(QString("最近更新: %1").arg(timeText));
}

bool WalletDialog::validateRechargeInput()
{
    QString amountStr = ui->rechargeAmountEdit->text().trimmed();

    if (amountStr.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入充值金额");
        ui->rechargeAmountEdit->setFocus();
        return false;
    }

    bool ok;
    double amount = amountStr.toDouble(&ok);

    if (!ok) {
        QMessageBox::warning(this, "输入错误", "请输入有效的金额");
        ui->rechargeAmountEdit->setFocus();
        ui->rechargeAmountEdit->selectAll();
        return false;
    }

    if (amount <= 0) {
        QMessageBox::warning(this, "输入错误", "充值金额必须大于0");
        ui->rechargeAmountEdit->setFocus();
        ui->rechargeAmountEdit->selectAll();
        return false;
    }

    if (amount > 10000) {
        QMessageBox::warning(this, "输入错误", "单次充值金额不能超过10000元");
        ui->rechargeAmountEdit->setFocus();
        ui->rechargeAmountEdit->selectAll();
        return false;
    }

    return true;
}

void WalletDialog::onRechargeButtonClicked()
{
    if (!validateRechargeInput()) {
        return;
    }

    if (!networkManager || !networkManager->isConnected()) {
        QMessageBox::warning(this, "错误", "未连接到服务器");
        return;
    }

    double amount = ui->rechargeAmountEdit->text().toDouble();

    // 发送充值请求
    NetworkMessage msg;
    msg.type = RECHARGE_REQUEST;
    msg.data["username"] = currentUsername;
    msg.data["amount"] = amount;

    networkManager->sendMessage(msg);

    // 禁用充值按钮，防止重复点击
    ui->rechargeButton->setEnabled(false);
    ui->rechargeButton->setText("处理中...");

    qDebug() << "发送充值请求，用户:" << currentUsername << "金额:" << amount;
}

void WalletDialog::onCloseButtonClicked()
{
    this->accept();
}

void WalletDialog::onMessageReceived(const NetworkMessage &message)
{
    if (message.type == WALLET_QUERY_RESPONSE) {
        bool success = message.data["success"].toBool();

        if (success) {
            double balance = message.data["balance"].toDouble();
            updateBalanceDisplay(balance);

            // 显示欢迎消息
            QString welcomeMsg = QString("欢迎您，%1！").arg(currentUsername);
            ui->welcomeLabel->setText(welcomeMsg);
        } else {
            QString errorMsg = message.data["message"].toString();
            QMessageBox::warning(this, "查询失败", errorMsg);
        }
    }
    else if (message.type == RECHARGE_RESPONSE) {
        // 恢复充值按钮状态
        ui->rechargeButton->setEnabled(true);
        ui->rechargeButton->setText("确认充值");

        bool success = message.data["success"].toBool();
        QString resultMsg = message.data["message"].toString();

        if (success) {
            double newBalance = message.data["new_balance"].toDouble();
            double rechargedAmount = message.data["recharged_amount"].toDouble();

            // 更新余额显示
            updateBalanceDisplay(newBalance);

            // 清空输入框
            ui->rechargeAmountEdit->clear();

            // 发射余额更新信号
            emit balanceUpdated();

            // 显示成功消息
            QString successMsg = QString(
                                     "✅ 充值成功！\n\n"
                                     "充值金额：¥%1\n"
                                     "当前余额：¥%2\n\n"
                                     "感谢您的支持！"
                                     ).arg(rechargedAmount, 0, 'f', 2)
                                     .arg(newBalance, 0, 'f', 2);

            QMessageBox::information(this, "充值成功", successMsg);
        } else {
            QMessageBox::warning(this, "充值失败", resultMsg);
        }
    }
}

void WalletDialog::onRechargeAmountChanged(const QString &text)
{
    // 实时计算充值后的余额
    if (!text.isEmpty()) {
        bool ok;
        double amount = text.toDouble(&ok);

        if (ok && amount > 0) {
            double newBalance = currentBalance + amount;
            QString newBalanceText = QString("¥%1").arg(newBalance, 0, 'f', 2);
            ui->newBalanceLabel->setText(QString("充值后余额: %1").arg(newBalanceText));

            // 设置字体颜色
            QPalette palette = ui->newBalanceLabel->palette();
            palette.setColor(QPalette::WindowText, QColor(46, 125, 50)); // 绿色
            ui->newBalanceLabel->setPalette(palette);
        } else {
            ui->newBalanceLabel->setText("充值后余额: --");
        }
    } else {
        ui->newBalanceLabel->setText("充值后余额: --");
    }
}
