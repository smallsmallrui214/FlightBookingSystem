#include "flightdetaildialog.h"
#include "ui_flightdetaildialog.h"
#include "bookingdialog.h"
#include "../Common/protocol.h"
#include <QMessageBox>
#include <QDebug>
#include <QTableWidgetItem>
#include <QPushButton>
#include <QHeaderView>  // 添加这个头文件
#include <QJsonArray>   // 添加这个头文件
#include <QJsonDocument> // 添加这个头文件

FlightDetailDialog::FlightDetailDialog(const Flight &flight,
                                       ClientNetworkManager *networkManager,
                                       const QString &username,
                                       QWidget *parent)
    : QDialog(parent), ui(new Ui::FlightDetailDialog),
    flight(flight), networkManager(networkManager), currentUsername(username)
{
    ui->setupUi(this);
    setupUI();

    // 连接信号
    connect(ui->closeButton, &QPushButton::clicked, this, &FlightDetailDialog::onCloseButtonClicked);
    connect(networkManager, &ClientNetworkManager::messageReceived,
            this, &FlightDetailDialog::onMessageReceived);

    // 加载舱位数据
    loadCabinData();
}

FlightDetailDialog::~FlightDetailDialog()
{
    delete ui;
}

void FlightDetailDialog::setupUI()
{
    // 设置窗口标题
    setWindowTitle(QString("航班详情 - %1").arg(flight.getFlightNumber()));

    // 显示航班信息
    ui->flightNumberLabel->setText(flight.getFlightNumber());
    ui->airlineLabel->setText(flight.getAirline());
    ui->aircraftTypeLabel->setText(flight.getAircraftType());

    // 显示航线信息
    ui->departureCityLabel->setText(flight.getDepartureCity());
    ui->departureTimeLabel->setText(flight.getDepartureTime().toString("hh:mm"));
    ui->arrivalCityLabel->setText(flight.getArrivalCity());
    ui->arrivalTimeLabel->setText(flight.getArrivalTime().toString("hh:mm"));
    ui->durationLabel->setText(flight.getDurationString());

    // 设置航班信息文本
    QString infoText = QString(
                           "✈️ 航班信息\n"
                           "航班号: %1\n"
                           "航空公司: %2\n"
                           "机型: %3\n\n"
                           "🛫 出发信息\n"
                           "城市: %4\n"
                           "时间: %5\n"
                           "日期: %6\n\n"
                           "🛬 到达信息\n"
                           "城市: %7\n"
                           "时间: %8\n"
                           "日期: %9\n\n"
                           "⏱️ 其他信息\n"
                           "飞行时长: %10\n"
                           "状态: %11\n"
                           "剩余座位: %12个"
                           ).arg(flight.getFlightNumber())
                           .arg(flight.getAirline())
                           .arg(flight.getAircraftType())
                           .arg(flight.getDepartureCity())
                           .arg(flight.getDepartureTime().toString("hh:mm"))
                           .arg(flight.getDepartureTime().toString("yyyy-MM-dd"))
                           .arg(flight.getArrivalCity())
                           .arg(flight.getArrivalTime().toString("hh:mm"))
                           .arg(flight.getArrivalTime().toString("yyyy-MM-dd"))
                           .arg(flight.getDurationString())
                           .arg(flight.getStatus())
                           .arg(flight.getAvailableSeats());

    ui->flightInfoText->setText(infoText);
}

void FlightDetailDialog::loadCabinData()
{
    if (!networkManager || !networkManager->isConnected()) {
        QMessageBox::warning(this, "错误", "未连接到服务器");
        return;
    }

    // 发送舱位查询请求
    NetworkMessage msg;
    msg.type = CABIN_SEARCH_REQUEST;
    msg.data["flight_id"] = flight.getId();

    networkManager->sendMessage(msg);
    qDebug() << "发送舱位查询请求，航班ID:" << flight.getId();
}

void FlightDetailDialog::displayCabinData(const QList<Cabin> &cabins)
{


    currentCabins = cabins;
    ui->cabinTable->setRowCount(cabins.size());

    for (int i = 0; i < cabins.size(); ++i) {
        const Cabin &cabin = cabins[i];

        // 舱位类型
        QTableWidgetItem *typeItem = new QTableWidgetItem(cabin.getCabinType());
        typeItem->setTextAlignment(Qt::AlignCenter);
        ui->cabinTable->setItem(i, 0, typeItem);

        // 价格
        QTableWidgetItem *priceItem = new QTableWidgetItem(QString("¥%1").arg(cabin.getPrice(), 0, 'f', 2));
        priceItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        priceItem->setForeground(Qt::red);
        ui->cabinTable->setItem(i, 1, priceItem);

        // 行李额度
        QTableWidgetItem *baggageItem = new QTableWidgetItem(cabin.getBaggageAllowance());
        baggageItem->setTextAlignment(Qt::AlignCenter);
        ui->cabinTable->setItem(i, 2, baggageItem);

        // 可用座位
        QTableWidgetItem *seatsItem = new QTableWidgetItem(
            QString("%1/%2").arg(cabin.getAvailableSeats()).arg(cabin.getTotalSeats()));
        seatsItem->setTextAlignment(Qt::AlignCenter);
        if (cabin.getAvailableSeats() < 5) {
            seatsItem->setForeground(Qt::red);
        } else if (cabin.getAvailableSeats() < 10) {
            seatsItem->setForeground(Qt::darkYellow);
        }
        ui->cabinTable->setItem(i, 3, seatsItem);

        // 预订按钮
        QPushButton *bookButton = new QPushButton("预订");
        bookButton->setStyleSheet(
            "QPushButton {"
            "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #ff5722, stop:1 #e64a19);"
            "    color: white;"
            "    border: none;"
            "    border-radius: 4px;"
            "    padding: 6px 12px;"
            "    font-size: 12px;"
            "    font-weight: bold;"
            "}"
            "QPushButton:hover {"
            "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #e64a19, stop:1 #d84315);"
            "}"
            "QPushButton:disabled {"
            "    background: #cccccc;"
            "    color: #888888;"
            "}"
            );
        bookButton->setEnabled(cabin.getAvailableSeats() > 0);

        connect(bookButton, &QPushButton::clicked, this, [this, i]() {
            onBookButtonClicked(i);
        });

        ui->cabinTable->setCellWidget(i, 4, bookButton);
    }

    // 调整列宽
    ui->cabinTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->cabinTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->cabinTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->cabinTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    ui->cabinTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
}

void FlightDetailDialog::onBookButtonClicked(int row)
{
    if (row >= 0 && row < currentCabins.size()) {
        const Cabin &cabin = currentCabins[row];
        showBookingDialog(cabin);
    }
}

void FlightDetailDialog::showBookingDialog(const Cabin &cabin)
{
    BookingDialog *dialog = new BookingDialog(flight, cabin, currentUsername,
                                              networkManager, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->exec();
}

void FlightDetailDialog::onCloseButtonClicked()
{
    this->accept();  // 关闭对话框
}

void FlightDetailDialog::onMessageReceived(const NetworkMessage &message)
{
    if (message.type == CABIN_SEARCH_RESPONSE) {
        bool success = message.data["success"].toBool();

        if (success) {
            QJsonArray cabinsArray = message.data["cabins"].toArray();
            QList<Cabin> cabins;

            for (auto it = cabinsArray.constBegin(); it != cabinsArray.constEnd(); ++it) {
                Cabin cabin = Cabin::fromJson((*it).toObject());
                cabins.append(cabin);
            }

            displayCabinData(cabins);
        } else {
            QMessageBox::warning(this, "错误", "加载舱位信息失败：" + message.data["message"].toString());
        }
    }
}
