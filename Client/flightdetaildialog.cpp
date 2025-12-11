#include "flightdetaildialog.h"
#include "ui_flightdetaildialog.h"
#include "bookingdialog.h"
#include "../Common/protocol.h"
#include <QMessageBox>
#include <QDebug>
#include <QTableWidgetItem>
#include <QPushButton>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonDocument>
#include <QHBoxLayout>

FlightDetailDialog::FlightDetailDialog(const Flight &flight,
                                       ClientNetworkManager *networkManager,
                                       const QString &username,
                                       QWidget *parent)
    : QDialog(parent), ui(new Ui::FlightDetailDialog),
    flight(flight), networkManager(networkManager), currentUsername(username),
    selectedCabinType(-1)  // 初始化为-1，表示未选择
{
    ui->setupUi(this);
    setupUI();

    // 连接信号
    connect(ui->closeButton, &QPushButton::clicked, this, &FlightDetailDialog::onCloseButtonClicked);
    connect(networkManager, &ClientNetworkManager::messageReceived,
            this, &FlightDetailDialog::onMessageReceived);

    // ======================= 添加的关键代码 =======================
    // 确保表格正确初始化
    ui->cabinTable->setShowGrid(true);
    ui->cabinTable->setGridStyle(Qt::SolidLine);
    ui->cabinTable->setAlternatingRowColors(true);
    ui->cabinTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->cabinTable->setSelectionMode(QAbstractItemView::SingleSelection);
    // ==========================================================

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
        aircraftDisplay += "（宽）";
    } else if (aircraftType == "A320" || aircraftType == "A319" ||
               aircraftType == "B737" || aircraftType == "ARJ21") {
        aircraftDisplay += "（窄）";
    } else {
        aircraftDisplay += "（中）";
    }

    ui->aircraftTypeLabel->setText(aircraftDisplay);

    // 显示航线信息
    ui->departureCityLabel->setText(flight.getDepartureCity());
    ui->departureTimeLabel->setText(flight.getDepartureTime().toString("hh:mm"));
    ui->arrivalCityLabel->setText(flight.getArrivalCity());
    ui->arrivalTimeLabel->setText(flight.getArrivalTime().toString("hh:mm"));
    ui->durationLabel->setText(flight.getDurationString());

    QString departureAirport = flight.getDepartureAirport();
    QString arrivalAirport = flight.getArrivalAirport();

    // 如果航班对象中没有机场信息，使用默认方法获取
    if (departureAirport.isEmpty()) {
        departureAirport = Flight::getDefaultAirport(flight.getDepartureCity());
    }
    if (arrivalAirport.isEmpty()) {
        arrivalAirport = Flight::getDefaultAirport(flight.getArrivalCity());
    }

    ui->departureAirportLabel->setText(departureAirport);
    ui->arrivalAirportLabel->setText(arrivalAirport);

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
                           .arg(aircraftDisplay)
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

    // 1. 缩小航班信息区域
    ui->flightInfoText->setStyleSheet(
        "font-size: 10px;"
        "padding: 5px;"
        "line-height: 1.2;"
        );

    // 2. 给购票表格预留更多空间
    ui->cabinTable->setMinimumHeight(350);

    // 3. 禁止表格内数据双击修改
    ui->cabinTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 4. 调整窗口大小，让表格有更多显示空间
    this->resize(700, 600);  // 增加窗口高度

    // 5. 在"舱位选择"标签页中，表格上面添加仓位选择按钮
    createCabinSelectionButtons();
}

void FlightDetailDialog::createCabinSelectionButtons()
{
    // 创建仓位选择按钮容器
    QWidget *cabinSelectionWidget = new QWidget();
    cabinSelectionWidget->setObjectName("cabinSelectionWidget");

    QHBoxLayout *cabinLayout = new QHBoxLayout(cabinSelectionWidget);
    cabinLayout->setContentsMargins(15, 15, 15, 15);
    cabinLayout->setSpacing(20);

    // 创建标题标签
    QLabel *titleLabel = new QLabel("选择仓位类型:");
    titleLabel->setStyleSheet(
        "QLabel {"
        "    font-size: 16px;"
        "    font-weight: bold;"
        "    color: #333;"
        "    font-family: 'Microsoft YaHei';"
        "}"
        );
    cabinLayout->addWidget(titleLabel);

    // 使用已有的cabinTypes来映射按钮类型
    // 1: 经济舱, 2: 商务舱, 3: 头等舱
    cabinTypes[1] = "经济舱";
    cabinTypes[2] = "商务舱";
    cabinTypes[3] = "头等舱";

    // 创建三个筛选按钮
    QPushButton *economyButton = new QPushButton(cabinTypes[1]);
    QPushButton *businessButton = new QPushButton(cabinTypes[2]);
    QPushButton *firstClassButton = new QPushButton(cabinTypes[3]);

    // 设置按钮属性，用于识别按钮类型
    economyButton->setProperty("cabinType", 1);
    businessButton->setProperty("cabinType", 2);
    firstClassButton->setProperty("cabinType", 3);

    // 设置按钮尺寸
    economyButton->setMinimumSize(120, 45);
    businessButton->setMinimumSize(120, 45);
    firstClassButton->setMinimumSize(120, 45);

    // 设置按钮样式
    QString normalStyle =
        "QPushButton {"
        "    background: white;"
        "    color: #333;"
        "    border: 2px solid #e0e0e0;"
        "    border-radius: 8px;"
        "    padding: 10px 20px;"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "    font-family: 'Microsoft YaHei';"
        "}"
        "QPushButton:hover {"
        "    background: #f5f5f5;"
        "    border-color: #1e88e5;"
        "}";

    QString selectedStyle =
        "QPushButton {"
        "    background: #e3f2fd;"
        "    color: #1e88e5;"
        "    border: 2px solid #1e88e5;"
        "    border-radius: 8px;"
        "    padding: 10px 20px;"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "    font-family: 'Microsoft YaHei';"
        "}"
        "QPushButton:hover {"
        "    background: #d3eafc;"
        "    border-color: #1565c0;"
        "}";

    // 默认选中经济舱
    selectedCabinType = 1;
    economyButton->setStyleSheet(selectedStyle);
    businessButton->setStyleSheet(normalStyle);
    firstClassButton->setStyleSheet(normalStyle);

    // 连接按钮点击信号
    connect(economyButton, &QPushButton::clicked, [this, economyButton, businessButton, firstClassButton, normalStyle, selectedStyle]() {
        selectedCabinType = 1;  // 经济舱
        economyButton->setStyleSheet(selectedStyle);
        businessButton->setStyleSheet(normalStyle);
        firstClassButton->setStyleSheet(normalStyle);

        // 重新显示数据
        if (!currentCabins.isEmpty()) {
            displayFilteredCabins();
        }
    });

    connect(businessButton, &QPushButton::clicked, [this, economyButton, businessButton, firstClassButton, normalStyle, selectedStyle]() {
        selectedCabinType = 2;  // 商务舱
        economyButton->setStyleSheet(normalStyle);
        businessButton->setStyleSheet(selectedStyle);
        firstClassButton->setStyleSheet(normalStyle);

        // 重新显示数据
        if (!currentCabins.isEmpty()) {
            displayFilteredCabins();
        }
    });

    connect(firstClassButton, &QPushButton::clicked, [this, economyButton, businessButton, firstClassButton, normalStyle, selectedStyle]() {
        selectedCabinType = 3;  // 头等舱
        economyButton->setStyleSheet(normalStyle);
        businessButton->setStyleSheet(normalStyle);
        firstClassButton->setStyleSheet(selectedStyle);

        // 重新显示数据
        if (!currentCabins.isEmpty()) {
            displayFilteredCabins();
        }
    });

    // 添加到布局
    cabinLayout->addWidget(economyButton);
    cabinLayout->addWidget(businessButton);
    cabinLayout->addWidget(firstClassButton);
    cabinLayout->addStretch();

    // 关键：将仓位选择按钮添加到"舱位选择"标签页的布局中（表格上面）
    // 获取cabinTab的布局（verticalLayout_3）
    QVBoxLayout *cabinTabLayout = qobject_cast<QVBoxLayout*>(ui->cabinTab->layout());
    if (cabinTabLayout) {
        // 在表格前面插入仓位选择widget
        cabinTabLayout->insertWidget(0, cabinSelectionWidget);
    }
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

    // 直接调用显示筛选后的数据
    displayFilteredCabins();

    // 添加额外的刷新代码
    if (ui->cabinTable->rowCount() > 0) {
        ui->cabinTable->viewport()->update();
        ui->cabinTable->repaint();
    }
}

void FlightDetailDialog::displayFilteredCabins()
{
    if (currentCabins.isEmpty() || selectedCabinType == -1) {
        return;
    }

    // 清空表格
    ui->cabinTable->setRowCount(0);

    // 根据selectedCabinType筛选舱位
    int rowCount = 0;

    for (const Cabin &cabin : currentCabins) {
        QString cabinType = cabin.getCabinType();
        bool shouldDisplay = false;

        // 根据选择的仓位类型筛选
        if (selectedCabinType == 1 && cabinType.contains("经济")) {
            shouldDisplay = true;
        } else if (selectedCabinType == 2 && cabinType.contains("商务")) {
            shouldDisplay = true;
        } else if (selectedCabinType == 3 && cabinType.contains("头等")) {
            shouldDisplay = true;
        }

        if (!shouldDisplay) {
            continue;
        }

        // 添加行
        ui->cabinTable->insertRow(rowCount);
        ui->cabinTable->setRowHeight(rowCount, 48);

        // 舱位类型 - 禁止编辑
        QTableWidgetItem *typeItem = new QTableWidgetItem(cabin.getCabinType());
        typeItem->setTextAlignment(Qt::AlignCenter);
        typeItem->setFont(QFont("Microsoft YaHei", 11));
        typeItem->setFlags(typeItem->flags() & ~Qt::ItemIsEditable);
        ui->cabinTable->setItem(rowCount, 0, typeItem);

        // 价格 - 禁止编辑
        double price = cabin.getPrice();
        QString priceText = QString("¥%1").arg(static_cast<int>(price));
        QTableWidgetItem *priceItem = new QTableWidgetItem(priceText);
        priceItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        priceItem->setForeground(QColor(220, 53, 69));
        priceItem->setFont(QFont("Microsoft YaHei", 12, QFont::Bold));
        priceItem->setFlags(priceItem->flags() & ~Qt::ItemIsEditable);
        ui->cabinTable->setItem(rowCount, 1, priceItem);

        // 行李额度 - 禁止编辑
        QTableWidgetItem *baggageItem = new QTableWidgetItem(cabin.getBaggageAllowance());
        baggageItem->setTextAlignment(Qt::AlignCenter);
        baggageItem->setFont(QFont("Microsoft YaHei", 11));
        baggageItem->setFlags(baggageItem->flags() & ~Qt::ItemIsEditable);
        ui->cabinTable->setItem(rowCount, 2, baggageItem);

        // 可用座位 - 禁止编辑
        int availableSeats = cabin.getAvailableSeats();
        int totalSeats = cabin.getTotalSeats();
        QString seatsText;

        if (availableSeats <= 0) {
            seatsText = "已售罄";
        } else if (availableSeats < 5) {
            seatsText = QString("仅剩%1座").arg(availableSeats);
        } else {
            seatsText = QString("%1/%2").arg(availableSeats).arg(totalSeats);
        }

        QTableWidgetItem *seatsItem = new QTableWidgetItem(seatsText);
        seatsItem->setTextAlignment(Qt::AlignCenter);
        seatsItem->setFont(QFont("Microsoft YaHei", 11));
        seatsItem->setFlags(seatsItem->flags() & ~Qt::ItemIsEditable);

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

        ui->cabinTable->setItem(rowCount, 3, seatsItem);

        // 预订按钮
        QString buttonText = availableSeats > 0 ? "预订" : "已售罄";
        QPushButton *bookButton = new QPushButton(buttonText);
        bookButton->setFixedSize(70, 34);

        if (availableSeats > 0) {
            bookButton->setStyleSheet(
                "QPushButton {"
                "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #28a745, stop:1 #20c997);"
                "    color: white;"
                "    border: none;"
                "    border-radius: 4px;"
                "    font-size: 12px;"
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
                "    font-size: 12px;"
                "    font-weight: bold;"
                "    font-family: 'Microsoft YaHei';"
                "}"
                );
        }

        bookButton->setEnabled(availableSeats > 0);

        connect(bookButton, &QPushButton::clicked, [this, rowCount]() {
            onBookButtonClicked(rowCount);
        });

        ui->cabinTable->setCellWidget(rowCount, 4, bookButton);

        rowCount++;
    }

    // 如果没有数据，显示提示
    if (rowCount == 0) {
        ui->cabinTable->setRowCount(1);
        ui->cabinTable->setRowHeight(0, 50);

        QTableWidgetItem *noDataItem = new QTableWidgetItem("该仓位暂无票源");
        noDataItem->setTextAlignment(Qt::AlignCenter);
        noDataItem->setForeground(QColor(108, 117, 125));
        noDataItem->setFont(QFont("Microsoft YaHei", 12));
        noDataItem->setFlags(noDataItem->flags() & ~Qt::ItemIsEditable);
        ui->cabinTable->setSpan(0, 0, 1, 5);
        ui->cabinTable->setItem(0, 0, noDataItem);
    }

    // 设置表格样式
    ui->cabinTable->horizontalHeader()->setStyleSheet(
        "QHeaderView::section {"
        "    background-color: #f8f9fa;"
        "    padding: 10px 8px;"
        "    border: 1px solid #dee2e6;"
        "    font-weight: bold;"
        "    font-size: 12px;"
        "    font-family: 'Microsoft YaHei';"
        "    color: #495057;"
        "}"
        );

    ui->cabinTable->setStyleSheet(
        "QTableWidget {"
        "    gridline-color: #dee2e6;"
        "    background-color: white;"
        "    alternate-background-color: #f8f9fa;"
        "    selection-background-color: #e3f2fd;"
        "}"
        "QTableWidget::item {"
        "    padding: 6px 8px;"
        "    border-bottom: 1px solid #f1f3f4;"
        "}"
        "QTableWidget::item:selected {"
        "    background-color: #e3f2fd;"
        "    color: #1e88e5;"
        "}"
        );

    ui->cabinTable->setAlternatingRowColors(true);

    // ======================= 添加的关键代码 =======================
    // 强制表格刷新显示
    ui->cabinTable->viewport()->update();

    // 调整列宽以适应内容
    ui->cabinTable->resizeColumnsToContents();

    // 可选：设置最小列宽，确保内容可见
    ui->cabinTable->horizontalHeader()->setMinimumSectionSize(80);

    // 可选：刷新整个表格，确保所有内容重绘
    ui->cabinTable->repaint();
    // ==========================================================
}

void FlightDetailDialog::onBookButtonClicked(int row)
{
    if (row >= 0 && row < ui->cabinTable->rowCount()) {
        // 获取当前显示的舱位
        QTableWidgetItem *typeItem = ui->cabinTable->item(row, 0);
        if (typeItem) {
            QString cabinType = typeItem->text();

            // 在currentCabins中查找对应的舱位
            for (const Cabin &cabin : currentCabins) {
                if (cabin.getCabinType() == cabinType) {
                    showBookingDialog(cabin);
                    return;
                }
            }
        }
    }
}

void FlightDetailDialog::showBookingDialog(const Cabin &cabin)
{
    BookingDialog *dialog = new BookingDialog(flight, cabin, currentUsername,
                                              networkManager, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    // 连接 bookingSuccess 信号
    connect(dialog, &BookingDialog::bookingSuccess, this, [this]() {
        qDebug() << "收到 bookingSuccess 信号，关闭详情窗口";

        // 关闭当前详情窗口
        this->accept();

    });
    dialog->exec();
}

void FlightDetailDialog::onCloseButtonClicked()
{
    this->accept();
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
