#include "bookingdialog.h"
#include "ui_bookingdialog.h"
#include "../Common/protocol.h"
#include <QMessageBox>
#include <QDebug>
#include <QRegularExpressionValidator>  // 修改：使用QRegularExpressionValidator
#include <QRegularExpression>

BookingDialog::BookingDialog(const Flight &flight, const Cabin &cabin,
                             const QString &username,
                             ClientNetworkManager *networkManager,
                             QWidget *parent)
    : QDialog(parent), ui(new Ui::BookingDialog),
    flight(flight), cabin(cabin), username(username),
    networkManager(networkManager)
{
    ui->setupUi(this);

    setWindowTitle(QString("机票预订 - %1").arg(flight.getFlightNumber()));

    // 设置输入验证 - 使用QRegularExpressionValidator
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

    // 连接信号
    connect(ui->bookButton, &QPushButton::clicked, this, &BookingDialog::onBookButtonClicked);
    connect(ui->cancelButton, &QPushButton::clicked, this, &BookingDialog::onCancelButtonClicked);
    connect(networkManager, &ClientNetworkManager::messageReceived,
            this, &BookingDialog::onMessageReceived);
}

BookingDialog::~BookingDialog()
{
    delete ui;
}

bool BookingDialog::validateInput()
{
    QString name = ui->nameEdit->text().trimmed();
    QString id = ui->idEdit->text().trimmed();
    QString phone = ui->phoneEdit->text().trimmed();

    if (name.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入乘客姓名");
        ui->nameEdit->setFocus();
        return false;
    }

    if (name.length() < 2 || name.length() > 20) {
        QMessageBox::warning(this, "输入错误", "姓名长度应在2-20个字符之间");
        ui->nameEdit->setFocus();
        ui->nameEdit->selectAll();
        return false;
    }

    if (id.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入身份证号码");
        ui->idEdit->setFocus();
        return false;
    }

    if (id.length() != 18) {
        QMessageBox::warning(this, "输入错误", "身份证号码应为18位");
        ui->idEdit->setFocus();
        ui->idEdit->selectAll();
        return false;
    }

    if (phone.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入手机号码");
        ui->phoneEdit->setFocus();
        return false;
    }

    if (phone.length() != 11) {
        QMessageBox::warning(this, "输入错误", "手机号码应为11位");
        ui->phoneEdit->setFocus();
        ui->phoneEdit->selectAll();
        return false;
    }

    return true;
}

void BookingDialog::onBookButtonClicked()
{
    if (!validateInput()) {
        return;
    }

    if (!networkManager || !networkManager->isConnected()) {
        QMessageBox::warning(this, "错误", "未连接到服务器");
        return;
    }

    // 检查舱位是否还有可用座位
    if (!cabin.getAvailableSeats()) {
        QMessageBox::warning(this, "预订失败", "该舱位已无可用座位");
        return;
    }

    // 发送预订请求
    NetworkMessage msg;
    msg.type = BOOKING_REQUEST;
    msg.data["flight_id"] = flight.getId();
    msg.data["cabin_id"] = cabin.getId();
    msg.data["user_id"] = 1; // 暂时使用默认用户ID
    msg.data["passenger_name"] = ui->nameEdit->text().trimmed();
    msg.data["passenger_id"] = ui->idEdit->text().trimmed().toUpper();
    msg.data["passenger_phone"] = ui->phoneEdit->text().trimmed();
    msg.data["total_price"] = cabin.getPrice();

    networkManager->sendMessage(msg);

    // 禁用预订按钮，防止重复点击
    ui->bookButton->setEnabled(false);
    ui->bookButton->setText("处理中...");
}

void BookingDialog::onCancelButtonClicked()
{
    reject();
}

void BookingDialog::onMessageReceived(const NetworkMessage &message)
{
    if (message.type == BOOKING_RESPONSE) {
        // 恢复按钮状态
        ui->bookButton->setEnabled(true);
        ui->bookButton->setText("确认预订");

        bool success = message.data["success"].toBool();
        QString resultMsg = message.data["message"].toString();

        if (success) {
            QString bookingNumber = message.data["booking_number"].toString();
            showSuccessMessage(bookingNumber);
            accept();
        } else {
            QMessageBox::warning(this, "预订失败", resultMsg);
        }
    }
}

void BookingDialog::showSuccessMessage(const QString &bookingNumber)
{
    QString successMsg = QString(
                             "✅ 预订成功！\n\n"
                             "订单号：%1\n"
                             "航班：%2 %3 → %4\n"
                             "舱位：%5\n"
                             "乘客：%6\n"
                             "总价：¥%7\n\n"
                             "请记下您的订单号，出行时请携带有效证件。"
                             ).arg(bookingNumber)
                             .arg(flight.getFlightNumber())
                             .arg(flight.getDepartureCity())
                             .arg(flight.getArrivalCity())
                             .arg(cabin.getCabinType())
                             .arg(ui->nameEdit->text().trimmed())
                             .arg(cabin.getPrice(), 0, 'f', 2);

    QMessageBox::information(this, "预订成功", successMsg);
}
