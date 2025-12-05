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


    // 判断飞机型号并添加分类
    QString aircraftType = flight.getAircraftType();
    QString aircraftDisplay = aircraftType;

    if (aircraftType == "A330" || aircraftType == "B787" || aircraftType == "B777") {
        aircraftDisplay += "（宽）";  // 宽体机
    } else if (aircraftType == "A320" || aircraftType == "A319" ||
               aircraftType == "B737" || aircraftType == "ARJ21") {
        aircraftDisplay += "（窄）";  // 窄体机
    } else {
        aircraftDisplay += "（中）";  // 中型机或其他
    }

    ui->aircraftTypeLabel->setText(aircraftDisplay);

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
                           .arg(aircraftDisplay)  // 使用带分类的机型
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

    // 先设置表格列宽和行高 - 增大整体尺寸
    ui->cabinTable->horizontalHeader()->setStretchLastSection(false);

    // 增大所有列宽，让表格看起来更舒适
    ui->cabinTable->setColumnWidth(0, 100);   // 舱位类型 - 加宽
    ui->cabinTable->setColumnWidth(1, 110);   // 价格 - 加宽
    ui->cabinTable->setColumnWidth(2, 100);   // 行李额度 - 加宽
    ui->cabinTable->setColumnWidth(3, 120);   // 可用座位 - 加宽
    ui->cabinTable->setColumnWidth(4, 100);   // 操作列 - 加到100px，按钮有足够空间

    for (int i = 0; i < cabins.size(); ++i) {
        const Cabin &cabin = cabins[i];

        // 设置合适的行高
        ui->cabinTable->setRowHeight(i, 48);  // 增加行高，让内容有呼吸空间

        // 舱位类型 - 使用稍大字体
        QTableWidgetItem *typeItem = new QTableWidgetItem(cabin.getCabinType());
        typeItem->setTextAlignment(Qt::AlignCenter);
        typeItem->setFont(QFont("Microsoft YaHei", 11));  // 增大字体
        ui->cabinTable->setItem(i, 0, typeItem);

        // 价格 - 只保留整数，但显示清晰
        double price = cabin.getPrice();
        QString priceText = QString("¥%1").arg(static_cast<int>(price));
        QTableWidgetItem *priceItem = new QTableWidgetItem(priceText);
        priceItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        priceItem->setForeground(QColor(220, 53, 69));
        priceItem->setFont(QFont("Microsoft YaHei", 12, QFont::Bold));  // 增大并加粗
        ui->cabinTable->setItem(i, 1, priceItem);

        // 行李额度 - 清晰显示
        QTableWidgetItem *baggageItem = new QTableWidgetItem(cabin.getBaggageAllowance());
        baggageItem->setTextAlignment(Qt::AlignCenter);
        baggageItem->setFont(QFont("Microsoft YaHei", 11));  // 增大字体
        ui->cabinTable->setItem(i, 2, baggageItem);

        // 可用座位 - 完整显示
        int availableSeats = cabin.getAvailableSeats();
        int totalSeats = cabin.getTotalSeats();
        QString seatsText;

        if (availableSeats <= 0) {
            seatsText = "已售罄";
        } else if (availableSeats < 5) {
            seatsText = QString("仅剩%1座").arg(availableSeats);  // 恢复完整文字
        } else {
            seatsText = QString("%1/%2").arg(availableSeats).arg(totalSeats);
        }

        QTableWidgetItem *seatsItem = new QTableWidgetItem(seatsText);
        seatsItem->setTextAlignment(Qt::AlignCenter);
        seatsItem->setFont(QFont("Microsoft YaHei", 11));

        // 根据座位情况设置颜色
        if (availableSeats <= 0) {
            seatsItem->setForeground(QColor(108, 117, 125));
        } else if (availableSeats < 5) {
            seatsItem->setForeground(QColor(220, 53, 69));
            seatsItem->setFont(QFont("Microsoft YaHei", 11, QFont::Bold));
        } else if (availableSeats < 10) {
            seatsItem->setForeground(QColor(255, 153, 0));
        } else {
            seatsItem->setForeground(QColor(40, 167, 69));
        }

        ui->cabinTable->setItem(i, 3, seatsItem);

        // 预订按钮 - 适当大小，有足够空间
        QString buttonText = availableSeats > 0 ? "预订" : "已售罄";
        QPushButton *bookButton = new QPushButton(buttonText);

        // 按钮大小适中，适合100px列宽
        bookButton->setFixedSize(85, 34);

        if (availableSeats > 0) {
            bookButton->setStyleSheet(
                "QPushButton {"
                "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #28a745, stop:1 #20c997);"
                "    color: white;"
                "    border: none;"
                "    border-radius: 4px;"   // 适当圆角
                "    font-size: 12px;"      // 增大字体
                "    font-weight: bold;"
                "    font-family: 'Microsoft YaHei';"
                "}"
                "QPushButton:hover {"
                "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #20c997, stop:1 #17a2b8);"
                "}"
                "QPushButton:pressed {"
                "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #1e7e34, stop:1 #138496);"
                "}"
                );
            bookButton->setCursor(Qt::PointingHandCursor);
        } else {
            bookButton->setStyleSheet(
                "QPushButton {"
                "    background: #6c757d;"
                "    color: white;"
                "    border: none;"
                "    border-radius: 4px;"
                "    font-size: 12px;"      // 增大字体
                "    font-weight: bold;"
                "    font-family: 'Microsoft YaHei';"
                "}"
                );
        }

        bookButton->setEnabled(availableSeats > 0);

        connect(bookButton, &QPushButton::clicked, this, [this, i]() {
            onBookButtonClicked(i);
        });

        ui->cabinTable->setCellWidget(i, 4, bookButton);
    }

    // 设置表格样式 - 更清晰的表头
    ui->cabinTable->horizontalHeader()->setStyleSheet(
        "QHeaderView::section {"
        "    background-color: #f8f9fa;"
        "    padding: 10px 8px;"           // 增加内边距
        "    border: 1px solid #dee2e6;"
        "    font-weight: bold;"
        "    font-size: 12px;"             // 增大表头字体
        "    font-family: 'Microsoft YaHei';"
        "    color: #495057;"
        "}"
        );

    // 设置表格整体样式
    ui->cabinTable->setStyleSheet(
        "QTableWidget {"
        "    gridline-color: #dee2e6;"
        "    background-color: white;"
        "    alternate-background-color: #f8f9fa;"  // 隔行变色
        "    selection-background-color: #e3f2fd;"
        "}"
        "QTableWidget::item {"
        "    padding: 6px 8px;"            // 增加格子内边距
        "    border-bottom: 1px solid #f1f3f4;"
        "}"
        "QTableWidget::item:selected {"
        "    background-color: #e3f2fd;"
        "    color: #1e88e5;"
        "}"
        );

    // 启用隔行变色
    ui->cabinTable->setAlternatingRowColors(true);
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
