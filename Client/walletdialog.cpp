#include "walletdialog.h"
#include "ui_walletdialog.h"
#include "rechargedialog.h"
#include "../Common/protocol.h"
#include <QMessageBox>
#include <QDebug>
#include <QDoubleValidator>
#include <QDateTime>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFont>
#include <QColor>
#include <QTimer>
#include <QHBoxLayout>
#include <QPushButton>

WalletDialog::WalletDialog(const QString &username,
                           ClientNetworkManager *networkManager,
                           double initialBalance,  // 添加初始余额参数
                           QWidget *parent)
    : QDialog(parent), ui(new Ui::WalletDialog),
    currentUsername(username), networkManager(networkManager),
    currentBalance(initialBalance)  // 使用传入的余额
{
    ui->setupUi(this);

    // 设置窗口标题
    setWindowTitle(QString("我的钱包 - %1").arg(username));
    setFixedSize(700, 600);  // 设置固定大小以显示所有内容

    // 设置充值记录表格
    ui->recordsTable->setColumnCount(4);
    QStringList headers;
    headers << "充值时间" << "充值金额" << "充值前余额" << "充值后余额";
    ui->recordsTable->setHorizontalHeaderLabels(headers);

    // 设置表格样式
    ui->recordsTable->horizontalHeader()->setStretchLastSection(true);
    ui->recordsTable->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    ui->recordsTable->horizontalHeader()->setStyleSheet(
        "QHeaderView::section {"
        "    background-color: #f8f9fa;"
        "    padding: 10px;"
        "    border: 1px solid #dee2e6;"
        "    font-weight: bold;"
        "    font-size: 12px;"
        "    color: #495057;"
        "    font-family: 'Microsoft YaHei';"
        "}"
        );

    ui->recordsTable->setStyleSheet(
        "QTableWidget {"
        "    gridline-color: #dee2e6;"
        "    background-color: white;"
        "    alternate-background-color: #f8f9fa;"
        "    border: 1px solid #dee2e6;"
        "    border-radius: 5px;"
        "}"
        "QTableWidget::item {"
        "    padding: 8px;"
        "    border-bottom: 1px solid #f1f3f4;"
        "    font-family: 'Microsoft YaHei';"
        "}"
        "QTableWidget::item:selected {"
        "    background-color: #e3f2fd;"
        "    color: #1e88e5;"
        "}"
        );

    ui->recordsTable->setAlternatingRowColors(true);
    ui->recordsTable->verticalHeader()->setVisible(false);
    ui->recordsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->recordsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->recordsTable->setSelectionMode(QAbstractItemView::SingleSelection);

    // 设置按钮样式
    QString buttonStyle =
        "QPushButton {"
        "    border: none;"
        "    border-radius: 6px;"
        "    padding: 10px 20px;"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "    font-family: 'Microsoft YaHei';"
        "    min-width: 120px;"
        "    min-height: 40px;"
        "}"
        "QPushButton:hover {"
        "    opacity: 0.9;"
        "}"
        "QPushButton:pressed {"
        "    opacity: 0.8;"
        "}"
        "QPushButton:disabled {"
        "    background: #bdc3c7;"
        "    color: #7f8c8d;"
        "}";

    // 设置具体按钮颜色
    ui->rechargeButton->setStyleSheet(buttonStyle +
                                      "QPushButton {"
                                      "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #28a745, stop:1 #20c997);"
                                      "    color: white;"
                                      "}");

    ui->viewRecordsButton->setStyleSheet(buttonStyle +
                                         "QPushButton {"
                                         "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #1E90FF, stop:1 #4169E1);"
                                         "    color: white;"
                                         "}");

    // 设置关闭按钮样式
    ui->closeButton->setStyleSheet(buttonStyle +
                                   "QPushButton {"
                                   "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #dc3545, stop:1 #c82333);"
                                   "    color: white;"
                                   "}");

    // 设置标签样式
    ui->welcomeLabel->setStyleSheet(
        "QLabel {"
        "    font-family: 'Microsoft YaHei';"
        "    color: #333;"
        "    font-size: 16px;"
        "    font-weight: bold;"
        "}");

    ui->lastUpdateLabel->setStyleSheet(
        "QLabel {"
        "    font-family: 'Microsoft YaHei';"
        "    color: #666;"
        "    font-size: 12px;"
        "}");

    ui->currentBalanceTitle->setStyleSheet(
        "QLabel {"
        "    font-family: 'Microsoft YaHei';"
        "    color: #333;"
        "    font-size: 16px;"
        "    font-weight: bold;"
        "}");

    ui->balanceLabel->setStyleSheet(
        "QLabel {"
        "    font-family: 'Microsoft YaHei';"
        "    color: #ff5722;"
        "    font-size: 20px;"
        "    font-weight: bold;"
        "}");

    // 设置GroupBox样式
    QString groupBoxStyle =
        "QGroupBox {"
        "    font-family: 'Microsoft YaHei';"
        "    border: 2px solid #e0e0e0;"
        "    border-radius: 10px;"
        "    margin-top: 10px;"
        "    padding-top: 10px;"
        "    background-color: white;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    subcontrol-position: top center;"
        "    padding: 0 10px;"
        "    background-color: white;"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "    color: #666;"
        "}";

    ui->welcomeGroupBox->setStyleSheet(groupBoxStyle);
    ui->recordsGroupBox->setStyleSheet(groupBoxStyle);

    // 关键修改：不再隐藏充值记录GroupBox，让它默认可见
    // ui->recordsGroupBox->setVisible(false); // 注释掉这行

    // 连接信号
    connect(ui->rechargeButton, &QPushButton::clicked, this, &WalletDialog::showRechargeDialog);
    connect(ui->viewRecordsButton, &QPushButton::clicked, this, &WalletDialog::loadRechargeRecords);
    connect(ui->closeButton, &QPushButton::clicked, this, &WalletDialog::accept);  // 使用accept关闭对话框

    // 关键修改：只连接充值相关的消息，不连接余额查询消息
    if (networkManager) {
        // 只连接充值记录和充值响应消息
        connect(networkManager, &ClientNetworkManager::messageReceived,
                this, &WalletDialog::onMessageReceived, Qt::UniqueConnection);
    }

    // 更新欢迎信息
    ui->welcomeLabel->setText(QString("欢迎您，%1！").arg(currentUsername));

    // 关键修改：直接显示传入的余额，不自己查询
    updateBalanceDisplay(initialBalance);

    qDebug() << "钱包对话框创建，使用初始余额:" << initialBalance << "，用户:" << currentUsername;

    // 只查询充值记录（余额已经有了）
    if (networkManager && networkManager->isConnected()) {
        loadRechargeRecords();
    }
}

WalletDialog::~WalletDialog()
{
    delete ui;
}

// 关键修改：删除 loadWalletInfo() 方法，因为我们不再自己查询余额
// 这个方法完全移除

void WalletDialog::refreshWalletInfo()
{
    qDebug() << "刷新钱包信息，用户:" << currentUsername;

    // 检查网络连接
    if (!networkManager || !networkManager->isConnected()) {
        ui->balanceLabel->setText("未连接");
        ui->balanceLabel->setStyleSheet(
            "QLabel {"
            "    font-family: 'Microsoft YaHei';"
            "    color: #dc3545;"
            "    font-size: 24px;"
            "    font-weight: bold;"
            "    padding: 5px;"
            "    border: 2px solid #dc3545;"
            "    border-radius: 8px;"
            "    background-color: #fff5f5;"
            "}");
        QMessageBox::warning(this, "网络错误", "请检查网络连接后重试");
        return;
    }

    // 显示查询状态
    ui->balanceLabel->setText("正在查询...");
    ui->balanceLabel->setStyleSheet(
        "QLabel {"
        "    font-family: 'Microsoft YaHei';"
        "    color: #1976d2;"
        "    font-size: 24px;"
        "    font-weight: bold;"
        "    padding: 5px;"
        "    border: 2px solid #1976d2;"
        "    border-radius: 8px;"
        "    background-color: #f0f8ff;"
        "}");

    // 发送钱包查询请求
    NetworkMessage msg;
    msg.type = WALLET_QUERY_REQUEST;
    msg.data["username"] = currentUsername;

    qDebug() << "发送刷新余额请求，消息类型:" << msg.type;
    qDebug() << "请求数据:" << QJsonDocument(msg.data).toJson();

    networkManager->sendMessage(msg);
}

void WalletDialog::loadRechargeRecords()
{
    if (!networkManager || !networkManager->isConnected()) {
        return;
    }

    // 发送充值记录查询请求
    NetworkMessage msg;
    msg.type = RECHARGE_RECORDS_REQUEST;
    msg.data["username"] = currentUsername;

    networkManager->sendMessage(msg);
    qDebug() << "发送充值记录查询请求，用户名:" << currentUsername;
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

    qDebug() << "更新余额显示:" << balanceText;
}

void WalletDialog::updateRechargeRecordsDisplay(const QJsonArray &records)
{
    ui->recordsTable->setRowCount(0);

    if (records.isEmpty()) {
        ui->recordsTable->setRowCount(1);
        QTableWidgetItem *noDataItem = new QTableWidgetItem("暂无充值记录");
        noDataItem->setTextAlignment(Qt::AlignCenter);
        noDataItem->setForeground(QColor(108, 117, 125));
        noDataItem->setFont(QFont("Microsoft YaHei", 12));
        ui->recordsTable->setSpan(0, 0, 1, 4);
        ui->recordsTable->setItem(0, 0, noDataItem);
        return;
    }

    // 按时间倒序显示（最近的在前）
    for (int i = 0; i < records.size(); i++) {
        QJsonObject record = records[i].toObject();

        QString timeStr = record["recharge_time"].toString();
        QDateTime rechargeTime = QDateTime::fromString(timeStr, Qt::ISODate);
        double amount = record["amount"].toDouble();
        double beforeBalance = record["before_balance"].toDouble();
        double afterBalance = record["after_balance"].toDouble();

        int row = ui->recordsTable->rowCount();
        ui->recordsTable->insertRow(row);

        // 设置行高
        ui->recordsTable->setRowHeight(row, 40);

        // 充值时间
        QString timeDisplay = rechargeTime.toString("yyyy-MM-dd\nhh:mm:ss");
        QTableWidgetItem *timeItem = new QTableWidgetItem(timeDisplay);
        timeItem->setTextAlignment(Qt::AlignCenter);
        timeItem->setFont(QFont("Microsoft YaHei", 10));
        ui->recordsTable->setItem(row, 0, timeItem);

        // 充值金额
        QTableWidgetItem *amountItem = new QTableWidgetItem(QString("¥%1").arg(amount, 0, 'f', 2));
        amountItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        amountItem->setForeground(QColor(40, 167, 69));  // 绿色
        amountItem->setFont(QFont("Microsoft YaHei", 11, QFont::Bold));
        ui->recordsTable->setItem(row, 1, amountItem);

        // 充值前余额
        QTableWidgetItem *beforeItem = new QTableWidgetItem(QString("¥%1").arg(beforeBalance, 0, 'f', 2));
        beforeItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        beforeItem->setFont(QFont("Microsoft YaHei", 10));
        ui->recordsTable->setItem(row, 2, beforeItem);

        // 充值后余额
        QTableWidgetItem *afterItem = new QTableWidgetItem(QString("¥%1").arg(afterBalance, 0, 'f', 2));
        afterItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        afterItem->setForeground(QColor(0, 123, 255));  // 蓝色
        afterItem->setFont(QFont("Microsoft YaHei", 11, QFont::Bold));
        ui->recordsTable->setItem(row, 3, afterItem);
    }

    // 调整列宽
    ui->recordsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->recordsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->recordsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->recordsTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
}

bool WalletDialog::validateRechargeInput(double amount)
{
    if (amount <= 0) {
        QMessageBox::warning(this, "输入错误", "充值金额必须大于0");
        return false;
    }

    if (amount > 10000) {
        QMessageBox::warning(this, "输入错误", "单次充值金额不能超过10000元");
        return false;
    }

    return true;
}

void WalletDialog::showRechargeDialog()
{
    qDebug() << "显示充值对话框，当前余额:" << currentBalance;

    // 创建充值对话框（只显示黄色方框内容）
    RechargeDialog *dialog = new RechargeDialog(currentBalance, this);

    // 连接确认充值信号
    connect(dialog, &RechargeDialog::rechargeConfirmed, this, &WalletDialog::processRechargeRequest);

    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->exec();
}

void WalletDialog::processRechargeRequest(double amount)
{
    qDebug() << "收到充值请求，金额:" << amount;

    // 验证输入
    if (!validateRechargeInput(amount)) {
        return;
    }

    if (!networkManager || !networkManager->isConnected()) {
        QMessageBox::warning(this, "错误", "未连接到服务器");
        return;
    }

    // 发送充值请求
    NetworkMessage msg;
    msg.type = RECHARGE_REQUEST;
    msg.data["username"] = currentUsername;
    msg.data["amount"] = amount;

    networkManager->sendMessage(msg);

    // 显示处理中提示
    QMessageBox::information(this, "充值处理中",
                             QString("正在处理 ¥%1 的充值请求...").arg(amount, 0, 'f', 2));

    qDebug() << "发送充值请求，用户:" << currentUsername << "金额:" << amount;
}

void WalletDialog::onMessageReceived(const NetworkMessage &message)
{
    qDebug() << "=== WalletDialog收到消息 ===";
    qDebug() << "消息类型:" << message.type;
    qDebug() << "WALLET_QUERY_RESPONSE值:" << WALLET_QUERY_RESPONSE;
    qDebug() << "消息数据:" << QJsonDocument(message.data).toJson();

    // 不再处理钱包查询响应，只处理充值相关消息
    // 钱包余额由MainWindow负责更新

    if (message.type == RECHARGE_RECORDS_RESPONSE) {
        bool success = message.data["success"].toBool();

        if (success) {
            QJsonArray records = message.data["records"].toArray();
            updateRechargeRecordsDisplay(records);
            qDebug() << "充值记录查询成功，记录数:" << records.size();
        } else {
            QString errorMsg = message.data["message"].toString();
            QMessageBox::warning(this, "查询失败", "无法获取充值记录：" + errorMsg);
            qDebug() << "充值记录查询失败:" << errorMsg;
        }
    }
    else if (message.type == RECHARGE_RESPONSE) {
        bool success = message.data["success"].toBool();
        QString resultMsg = message.data["message"].toString();

        if (success) {
            double newBalance = message.data["new_balance"].toDouble();
            double rechargedAmount = message.data["recharged_amount"].toDouble();

            // 更新余额显示
            updateBalanceDisplay(newBalance);

            // 刷新充值记录
            loadRechargeRecords();

            // 发射余额更新信号，通知MainWindow刷新
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
            qDebug() << "充值成功，新余额:" << newBalance << "充值金额:" << rechargedAmount;
        } else {
            QMessageBox::warning(this, "充值失败", resultMsg);
            qDebug() << "充值失败:" << resultMsg;
        }
    }
}
