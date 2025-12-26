#include "bookingdialog.h"
#include "ui_bookingdialog.h"
#include "../Common/protocol.h"
#include "walletdialog.h"
#include <QMessageBox>
#include <QDebug>
#include <QRegularExpressionValidator>
#include <QRegularExpression>

BookingDialog::BookingDialog(const Flight &flight, const Cabin &cabin,
                             const QString &username,
                             ClientNetworkManager *networkManager,
                             QWidget *parent)
    : QDialog(parent), ui(new Ui::BookingDialog),
    flight(flight), cabin(cabin), username(username),
    networkManager(networkManager),
    userBalance(0.0), balanceChecked(false), isBalanceSufficient(false)
{
    ui->setupUi(this);

    // 设置窗口标题
    setWindowTitle(QString("机票预订 - %1").arg(flight.getFlightNumber()));

    // 设置输入验证
    QRegularExpression idRegex("^\\d{17}[\\dXx]$");
    ui->idEdit->setValidator(new QRegularExpressionValidator(idRegex, this));

    QRegularExpression phoneRegex("^1[3-9]\\d{9}$");
    ui->phoneEdit->setValidator(new QRegularExpressionValidator(phoneRegex, this));

    // 显示航班和舱位信息
    QString flightInfo = QString("%1 %2 → %3\n%4 ¥%5")
                             .arg(flight.getFlightNumber())
                             .arg(flight.getDepartureCity())
                             .arg(flight.getArrivalCity())
                             .arg(cabin.getCabinType())
                             .arg(cabin.getPrice(), 0, 'f', 2);

    ui->flightInfoLabel->setText(flightInfo);
    ui->totalPriceLabel->setText(QString("总价：¥%1").arg(cabin.getPrice(), 0, 'f', 2));

    // 初始化UI状态
    ui->balanceLabel->setText("正在查询余额...");
    ui->balanceStatusLabel->setText("");
    ui->balanceStatusLabel->setStyleSheet("");
    ui->rechargeButton->setVisible(false);
    ui->bookButton->setEnabled(false);
    ui->statusLabel->setText("请填写完整的乘客信息");

    // 连接信号
    connect(ui->bookButton, &QPushButton::clicked, this, &BookingDialog::onBookButtonClicked);
    connect(ui->cancelButton, &QPushButton::clicked, this, &BookingDialog::onCancelButtonClicked);
    connect(ui->rechargeButton, &QPushButton::clicked, this, &BookingDialog::onRechargeButtonClicked);

    if (networkManager) {
        connect(networkManager, &ClientNetworkManager::messageReceived,
                this, &BookingDialog::onMessageReceived);
    }

    // 启动输入检查定时器
    inputCheckTimer = new QTimer(this);
    inputCheckTimer->setInterval(300);
    connect(inputCheckTimer, &QTimer::timeout, this, &BookingDialog::checkInputValidity);
    inputCheckTimer->start();

    // 查询用户余额
    queryWalletBalance();
}

BookingDialog::~BookingDialog()
{
    delete ui;
}

void BookingDialog::queryWalletBalance()
{
    if (!networkManager || !networkManager->isConnected()) {
        QMessageBox::warning(this, "错误", "未连接到服务器");
        ui->balanceLabel->setText("无法连接服务器");
        return;
    }

    NetworkMessage msg;
    msg.type = WALLET_QUERY_REQUEST;
    msg.data["username"] = username;

    networkManager->sendMessage(msg);
    qDebug() << "查询用户余额:" << username;
}

void BookingDialog::updateBalanceDisplay()
{
    QString balanceText = QString("¥%1").arg(userBalance, 0, 'f', 2);
    ui->balanceLabel->setText(balanceText);

    double price = cabin.getPrice();
    isBalanceSufficient = (userBalance >= price);

    if (isBalanceSufficient) {
        ui->balanceStatusLabel->setText("✅ 余额充足");
        ui->balanceStatusLabel->setStyleSheet("color: green; font-weight: bold; background-color: #e8f5e8; padding: 4px 8px; border-radius: 4px;");
        ui->rechargeButton->setVisible(false);
    } else {
        double needed = price - userBalance;
        ui->balanceStatusLabel->setText(QString("❌ 余额不足，还需 ¥%1").arg(needed, 0, 'f', 2));
        ui->balanceStatusLabel->setStyleSheet("color: #d32f2f; font-weight: bold; background-color: #ffebee; padding: 4px 8px; border-radius: 4px;");
        ui->rechargeButton->setVisible(true);
    }

    // 更新UI状态
    updateUIState();
}

void BookingDialog::checkInputValidity()
{
    updateUIState();
}

void BookingDialog::updateUIState()
{
    bool inputValid = validateInput();
    bool canBook = inputValid && balanceChecked && isBalanceSufficient;

    ui->bookButton->setEnabled(canBook);

    // 更新状态提示
    if (!inputValid) {
        ui->statusLabel->setText("请填写完整的乘客信息");
        ui->statusLabel->setStyleSheet("color: orange; background-color: #fff3e0;");
    } else if (!balanceChecked) {
        ui->statusLabel->setText("正在查询余额...");
        ui->statusLabel->setStyleSheet("color: #1976d2; background-color: #e3f2fd;");
    } else if (!isBalanceSufficient) {
        ui->statusLabel->setText("余额不足，请先充值");
        ui->statusLabel->setStyleSheet("color: #d32f2f; background-color: #ffebee;");
    } else {
        ui->statusLabel->setText("信息完整，可以预订");
        ui->statusLabel->setStyleSheet("color: #388e3c; background-color: #e8f5e8;");
    }
}

bool BookingDialog::validateInput()
{
    QString name = ui->nameEdit->text().trimmed();
    QString id = ui->idEdit->text().trimmed();
    QString phone = ui->phoneEdit->text().trimmed();

    if (name.isEmpty()) {
        return false;
    }

    if (name.length() < 2 || name.length() > 20) {
        return false;
    }

    if (id.isEmpty()) {
        return false;
    }

    if (id.length() != 18) {
        return false;
    }

    if (phone.isEmpty()) {
        return false;
    }

    if (phone.length() != 11) {
        return false;
    }

    return true;
}

void BookingDialog::onRechargeButtonClicked()
{
    // 创建充值对话框
    WalletDialog *dialog = new WalletDialog(username, networkManager, userBalance, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);

    // 连接余额更新信号
    connect(dialog, &WalletDialog::balanceUpdated, this, [this]() {
        qDebug() << "收到余额更新信号";

        // 显示充值成功提示
        QMessageBox::information(this, "充值成功",
                                 "充值已完成！系统正在更新您的余额信息...");

        // 延迟查询，给服务器一点时间处理
        QTimer::singleShot(1000, this, [this]() {
            queryWalletBalance();
        });
    });

    dialog->exec();
}

void BookingDialog::processBooking()
{
    if (!networkManager || !networkManager->isConnected()) {
        QMessageBox::warning(this, "错误", "未连接到服务器");
        return;
    }

    // 检查舱位是否还有可用座位
    if (cabin.getAvailableSeats() <= 0) {
        QMessageBox::warning(this, "预订失败", "该舱位已无可用座位");
        return;
    }

    // 发送预订请求
    NetworkMessage msg;
    msg.type = BOOKING_REQUEST;
    msg.data["flight_id"] = flight.getId();
    msg.data["cabin_id"] = cabin.getId();
    msg.data["username"] = username;
    msg.data["passenger_name"] = ui->nameEdit->text().trimmed();
    msg.data["passenger_id"] = ui->idEdit->text().trimmed().toUpper();
    msg.data["passenger_phone"] = ui->phoneEdit->text().trimmed();
    msg.data["total_price"] = cabin.getPrice();

    networkManager->sendMessage(msg);

    // 禁用预订按钮，防止重复点击
    ui->bookButton->setEnabled(false);
    ui->bookButton->setText("处理中...");
}

void BookingDialog::onBookButtonClicked()
{
    if (!validateInput()) {
        QMessageBox::warning(this, "输入错误", "请填写完整且正确的乘客信息");
        return;
    }

    if (!balanceChecked) {
        QMessageBox::warning(this, "错误", "正在查询余额，请稍后...");
        return;
    }

    if (!isBalanceSufficient) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "余额不足",
                                      QString("您的余额不足！\n\n需要支付：¥%1\n当前余额：¥%2\n差额：¥%3\n\n是否前往充值？")
                                          .arg(cabin.getPrice(), 0, 'f', 2)
                                          .arg(userBalance, 0, 'f', 2)
                                          .arg(cabin.getPrice() - userBalance, 0, 'f', 2),
                                      QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes) {
            onRechargeButtonClicked();
        }
        return;
    }

    // 确认对话框
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "确认预订",
                                  QString("确认预订本次航班吗？\n\n"
                                          "航班：%1 %2 → %3\n"
                                          "舱位：%4\n"
                                          "乘客：%5\n"
                                          "总价：¥%6\n"
                                          "当前余额：¥%7\n\n"
                                          "确认后将扣除相应金额")
                                      .arg(flight.getFlightNumber())
                                      .arg(flight.getDepartureCity())
                                      .arg(flight.getArrivalCity())
                                      .arg(cabin.getCabinType())
                                      .arg(ui->nameEdit->text().trimmed())
                                      .arg(cabin.getPrice(), 0, 'f', 2)
                                      .arg(userBalance, 0, 'f', 2),
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) {
        return;
    }

    processBooking();
}

void BookingDialog::onCancelButtonClicked()
{
    reject();
}

void BookingDialog::onMessageReceived(const NetworkMessage &message)
{
    if (message.type == WALLET_QUERY_RESPONSE) {
        bool success = message.data["success"].toBool();
        if (success) {
            userBalance = message.data["balance"].toDouble();
            balanceChecked = true;
            qDebug() << "查询到用户余额:" << username << "余额:" << userBalance;
            updateBalanceDisplay();
        } else {
            ui->balanceLabel->setText("查询失败");
            QMessageBox::warning(this, "查询失败",
                                 "无法查询钱包余额：" + message.data["message"].toString());
        }
    }
    else if (message.type == BOOKING_RESPONSE) {
        // 恢复按钮状态
        ui->bookButton->setEnabled(true);
        ui->bookButton->setText("预订");

        bool success = message.data["success"].toBool();
        QString resultMsg = message.data["message"].toString();

        if (success) {
            QString bookingNumber = message.data["booking_number"].toString();
            double newBalance = message.data["new_balance"].toDouble();

            // 更新余额显示
            userBalance = newBalance;
            updateBalanceDisplay();

            // 显示成功消息
            QString successMsg = QString(
                                     "✅ 预订成功！\n\n"
                                     "订单号：%1\n"
                                     "航班：%2 %3 → %4\n"
                                     "舱位：%5\n"
                                     "乘客：%6\n"
                                     "总价：¥%7\n"
                                     "支付后余额：¥%8\n\n"
                                     "请记下您的订单号，出行时请携带有效证件。"
                                     ).arg(bookingNumber)
                                     .arg(flight.getFlightNumber())
                                     .arg(flight.getDepartureCity())
                                     .arg(flight.getArrivalCity())
                                     .arg(cabin.getCabinType())
                                     .arg(ui->nameEdit->text().trimmed())
                                     .arg(cabin.getPrice(), 0, 'f', 2)
                                     .arg(newBalance, 0, 'f', 2);

            // 使用 QMessageBox 的标准按钮，确保对话框能关闭
            QMessageBox msgBox(this);
            msgBox.setWindowTitle("预订成功");
            msgBox.setText(successMsg);
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setDefaultButton(QMessageBox::Ok);
            msgBox.setIcon(QMessageBox::Information);

            // 连接按钮点击信号
            connect(&msgBox, &QMessageBox::finished, this, [this]() {
                // 无论如何都关闭对话框
                this->accept();
            });

            msgBox.exec();

            // 发送预订成功信号
            emit bookingSuccess();

        } else {
            QMessageBox::warning(this, "预订失败", resultMsg);

            // 如果是余额问题，重新查询余额
            if (resultMsg.contains("余额") || resultMsg.contains("不足")) {
                queryWalletBalance();
            }
        }
    }
}
