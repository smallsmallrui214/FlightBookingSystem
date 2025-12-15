#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "../Common/protocol.h"
#include "calendardialog.h"
#include "flightdetaildialog.h"
#include "walletdialog.h"
#include "changeUsernameDialog.h"
#include "changepassworddialog.h"
#include <QListWidgetItem>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QDebug>
#include <QDate>
#include <QTimer>
#include <QJsonArray>
#include <QJsonDocument>
#include <QButtonGroup>
#include <QInputDialog>
#include <QCloseEvent>
#include <QAbstractButton>
#include <QString>
#include <QColor>
#include <QBrush>
#include <QWidget>
#include <QMovie>

// 自定义航班列表项Widget
class FlightItemWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FlightItemWidget(const Flight &flight, QWidget *parent = nullptr);
    Flight getFlight() const { return flight; }

signals:
    void bookRequested(const Flight &flight);

private slots:
    void onBookButtonClicked()
    {
        emit bookRequested(flight);
    }

private:
    Flight flight;
};

// FlightItemWidget 的实现
FlightItemWidget::FlightItemWidget(const Flight &flight, QWidget *parent)
    : QWidget(parent), flight(flight)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(20, 15, 20, 15);
    layout->setSpacing(15);

    // 左侧：航班基本信息
    QVBoxLayout *infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(8);

    // 航班号和航空公司
    QHBoxLayout *headerLayout = new QHBoxLayout();
    QLabel *flightNumberLabel = new QLabel(flight.getFlightNumber());
    flightNumberLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #1e88e5;");

    QLabel *airlineLabel = new QLabel(flight.getAirline());
    airlineLabel->setStyleSheet("font-size: 14px; color: #666; background: #f0f7ff; padding: 4px 8px; border-radius: 4px;");

    // 判断飞机型号并添加宽窄分类
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

    QLabel *aircraftLabel = new QLabel(aircraftDisplay);
    aircraftLabel->setStyleSheet("font-size: 12px; color: #666; background: #f5f5f5; padding: 2px 6px; border-radius: 3px;");

    headerLayout->addWidget(flightNumberLabel);
    headerLayout->addWidget(airlineLabel);
    headerLayout->addWidget(aircraftLabel);
    headerLayout->addStretch();

    // 时间和城市信息
    QHBoxLayout *timeLayout = new QHBoxLayout();

    // 出发信息
    QVBoxLayout *departureLayout = new QVBoxLayout();
    QLabel *departureTimeLabel = new QLabel(flight.getDepartureTime().toString("hh:mm"));
    departureTimeLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #333;");
    QLabel *departureCityLabel = new QLabel(flight.getDepartureCity());
    departureCityLabel->setStyleSheet("font-size: 14px; color: #666;");
    departureLayout->addWidget(departureTimeLabel);
    departureLayout->addWidget(departureCityLabel);

    // 箭头和时长
    QVBoxLayout *arrowLayout = new QVBoxLayout();
    QLabel *arrowLabel = new QLabel("→");
    arrowLabel->setStyleSheet("font-size: 16px; color: #999; padding: 10px 5px;");
    QLabel *durationLabel = new QLabel(flight.getDurationString());
    durationLabel->setStyleSheet("font-size: 12px; color: #999;");
    arrowLayout->addWidget(arrowLabel);
    arrowLayout->addWidget(durationLabel);
    arrowLayout->setAlignment(Qt::AlignCenter);

    // 到达信息
    QVBoxLayout *arrivalLayout = new QVBoxLayout();
    QLabel *arrivalTimeLabel = new QLabel(flight.getArrivalTime().toString("hh:mm"));
    arrivalTimeLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #333;");
    QLabel *arrivalCityLabel = new QLabel(flight.getArrivalCity());
    arrivalCityLabel->setStyleSheet("font-size: 14px; color: #666;");
    arrivalLayout->addWidget(arrivalTimeLabel);
    arrivalLayout->addWidget(arrivalCityLabel);

    timeLayout->addLayout(departureLayout);
    timeLayout->addLayout(arrowLayout);
    timeLayout->addLayout(arrivalLayout);
    timeLayout->addStretch();

    infoLayout->addLayout(headerLayout);
    infoLayout->addLayout(timeLayout);

    // 右侧：价格和预订按钮
    QVBoxLayout *priceLayout = new QVBoxLayout();
    priceLayout->setAlignment(Qt::AlignRight | Qt::AlignTop);
    priceLayout->setSpacing(8);

    // 价格 - 只保留整数，添加"起"字
    double price = flight.getPrice();
    QString priceText = QString("¥%1起").arg(static_cast<int>(price));
    QLabel *priceLabel = new QLabel(priceText);
    priceLabel->setStyleSheet(
        "font-size: 24px; "
        "font-weight: bold; "
        "color: #ff5722; "
        "font-family: 'Microsoft YaHei';"
        );

    // 添加价格说明小字
    QLabel *priceNoteLabel = new QLabel("经济舱最低价");
    priceNoteLabel->setStyleSheet("font-size: 11px; color: #999;");

    // 剩余座位信息
    int availableSeats = flight.getAvailableSeats();
    QString seatsText;
    QString seatsStyle;

    if (availableSeats <= 0) {
        seatsText = "已售罄";
        seatsStyle = "font-size: 12px; color: #9e9e9e; padding: 2px 8px; background: #f5f5f5; border-radius: 3px;";
    } else if (availableSeats < 5) {
        seatsText = QString("仅剩%1张").arg(availableSeats);
        seatsStyle = "font-size: 12px; color: #f44336; font-weight: bold; padding: 2px 8px; background: #ffebee; border-radius: 3px;";
    } else if (availableSeats < 20) {
        seatsText = QString("余%1张").arg(availableSeats);
        seatsStyle = "font-size: 12px; color: #ff9800; padding: 2px 8px; background: #fff3e0; border-radius: 3px;";
    } else {
        seatsText = QString("余%1张").arg(availableSeats);
        seatsStyle = "font-size: 12px; color: #4caf50; padding: 2px 8px; background: #e8f5e8; border-radius: 3px;";
    }

    QLabel *seatsLabel = new QLabel(seatsText);
    seatsLabel->setStyleSheet(seatsStyle);

    QPushButton *bookButton = new QPushButton(availableSeats > 0 ? "查看详情" : "已售罄");

    if (availableSeats > 0) {
        bookButton->setStyleSheet(
            "QPushButton {"
            "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #1e88e5, stop:1 #1565c0);"
            "    color: white;"
            "    border: none;"
            "    border-radius: 6px;"
            "    padding: 10px 24px;"
            "    font-size: 14px;"
            "    font-weight: bold;"
            "    font-family: 'Microsoft YaHei';"
            "    min-width: 100px;"
            "}"
            "QPushButton:hover {"
            "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #1565c0, stop:1 #0d47a1);"
            "}"
            "QPushButton:pressed {"
            "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #0d47a1, stop:1 #002171);"
            "}"
            );
        bookButton->setCursor(Qt::PointingHandCursor);
    } else {
        bookButton->setStyleSheet(
            "QPushButton {"
            "    background: #bdbdbd;"
            "    color: white;"
            "    border: none;"
            "    border-radius: 6px;"
            "    padding: 10px 24px;"
            "    font-size: 14px;"
            "    font-weight: bold;"
            "    font-family: 'Microsoft YaHei';"
            "    min-width: 100px;"
            "}"
            );
        bookButton->setCursor(Qt::ForbiddenCursor);
    }

    connect(bookButton, &QPushButton::clicked, this, &FlightItemWidget::onBookButtonClicked);

    priceLayout->addWidget(priceLabel);
    priceLayout->addWidget(priceNoteLabel);
    priceLayout->addWidget(seatsLabel);
    priceLayout->addWidget(bookButton);

    layout->addLayout(infoLayout, 1);
    layout->addLayout(priceLayout);

    // 美化整体样式
    setStyleSheet(
        "FlightItemWidget {"
        "    background: white;"
        "    border: 1px solid #e0e0e0;"
        "    border-radius: 10px;"
        "    margin: 5px 15px;"
        "}"
        "FlightItemWidget:hover {"
        "    background: #f8fdff;"
        "    border-color: #bbdefb;"
        "    box-shadow: 0 2px 8px rgba(30, 136, 229, 0.1);"
        "}"
        );
}

// MainWindow 的实现
MainWindow::MainWindow(const QString &username, ClientNetworkManager* networkManager, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), currentUsername(username), networkManager(networkManager),
    userWalletBalance(0.0), walletBalanceChecked(false),
    dateButtonGroup(nullptr), navButtonGroup(nullptr)
{
    ui->setupUi(this);

    // 设置订单列表样式
    ui->ordersListWidget->setStyleSheet(
        "QListWidget {"
        "    background: #f8f9fa;"
        "    border: 1px solid #dee2e6;"
        "    border-radius: 5px;"
        "    outline: 0;"
        "}"
        "QListWidget::item {"
        "    border-bottom: 1px solid #e9ecef;"
        "    padding: 2px;"
        "}"
        "QListWidget::item:selected {"
        "    background: #e3f2fd;"
        "    color: #1e88e5;"
        "}"
        "QListWidget::item:hover {"
        "    background: #f1f8ff;"
        "}"
        );

    // 设置用户信息
    ui->userNameLabel->setText(QString("欢迎，%1").arg(username));

    // 设置钱包余额显示为查询中状态
    ui->balanceLabel->setText("查询中...");

    // 设置默认城市
    ui->departureEdit->setText("广州");
    ui->arrivalEdit->setText("宜宾");

    qDebug() << "主窗口初始化完成";

    // 初始化日期选择系统
    setupDateSelection();

    // 初始化导航系统
    setupNavigation();

    setupConnections();

    // 延迟搜索，确保界面完全加载
    QTimer::singleShot(500, this, &MainWindow::onSearchButtonClicked);
}

void MainWindow::setupDateSelection()
{
    // 初始化日期按钮组
    dateButtonGroup = new QButtonGroup(this);
    dateButtonGroup->setExclusive(true);

    // 收集所有日期按钮
    dateButtons.clear();
    dateButtons.append(ui->dateButton_0);
    dateButtons.append(ui->dateButton_1);
    dateButtons.append(ui->dateButton_2);
    dateButtons.append(ui->dateButton_3);
    dateButtons.append(ui->dateButton_4);
    dateButtons.append(ui->dateButton_5);
    dateButtons.append(ui->dateButton_6);

    // 将按钮添加到按钮组
    for (int i = 0; i < dateButtons.size(); ++i) {
        dateButtonGroup->addButton(dateButtons[i], i);
    }

    // 设置初始日期为今天
    selectedDate = QDate::currentDate();
    currentStartDate = QDate::currentDate();

    // 初始显示从今天开始的7天
    updateDateButtons();

    // 初始禁用向左按钮
    ui->prevWeekButton->setEnabled(false);
}

void MainWindow::updateDateButtons()
{
    QDate currentDate = QDate::currentDate();

    for (int i = 0; i < dateButtons.size(); ++i) {
        QDate buttonDate = currentStartDate.addDays(i);
        QPushButton *button = dateButtons[i];

        // 设置日期文本
        QString dayName;
        if (buttonDate == currentDate) {
            dayName = "今天";
        } else if (buttonDate == currentDate.addDays(1)) {
            dayName = "明天";
        } else if (buttonDate == currentDate.addDays(2)) {
            dayName = "后天";
        } else {
            QStringList weekDays = {"周日", "周一", "周二", "周三", "周四", "周五", "周六"};
            dayName = weekDays[buttonDate.dayOfWeek() - 1];
        }

        QString buttonText = QString("%1\n%2").arg(dayName, buttonDate.toString("MM/dd"));
        button->setText(buttonText);

        // 设置选中状态
        bool isSelected = (buttonDate == selectedDate);
        bool isToday = (buttonDate == currentDate);

        // 更新按钮样式
        if (isSelected) {
            button->setStyleSheet(
                "QPushButton {"
                "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #1e88e5, stop:1 #1565c0);"
                "    color: white;"
                "    border: 2px solid #0d47a1;"
                "    border-radius: 6px;"
                "    padding: 5px 8px;"
                "    font-size: 11px;"
                "    font-weight: bold;"
                "    min-width: 70px;"
                "    max-width: 70px;"
                "    min-height: 40px;"
                "    max-height: 40px;"
                "}"
                "QPushButton:hover {"
                "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #1976d2, stop:1 #0d47a1);"
                "}"
                );
        } else if (isToday) {
            button->setStyleSheet(
                "QPushButton {"
                "    background: #e8f5e8;"
                "    color: #2e7d32;"
                "    border: 2px solid #4caf50;"
                "    border-radius: 6px;"
                "    padding: 5px 8px;"
                "    font-size: 11px;"
                "    font-weight: bold;"
                "    min-width: 70px;"
                "    max-width: 70px;"
                "    min-height: 40px;"
                "    max-height: 40px;"
                "}"
                "QPushButton:hover {"
                "    background: #c8e6c9;"
                "}"
                );
        } else {
            button->setStyleSheet(
                "QPushButton {"
                "    background: white;"
                "    color: #333;"
                "    border: 2px solid #e0e0e0;"
                "    border-radius: 6px;"
                "    padding: 5px 8px;"
                "    font-size: 11px;"
                "    min-width: 70px;"
                "    max-width: 70px;"
                "    min-height: 40px;"
                "    max-height: 40px;"
                "}"
                "QPushButton:hover {"
                "    background: #f5f5f5;"
                "    border-color: #1e88e5;"
                "}"
                "QPushButton:checked {"
                "    background: #e3f2fd;"
                "    border-color: #1e88e5;"
                "    color: #1e88e5;"
                "}"
                );
        }

        button->setEnabled(true);
        button->setChecked(isSelected);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    emit logoutRequested();
}

void MainWindow::setupConnections()
{
    connect(ui->searchButton, &QPushButton::clicked, this, &MainWindow::onSearchButtonClicked);
    connect(ui->flightListWidget, &QListWidget::itemDoubleClicked, this, &MainWindow::onFlightItemDoubleClicked);
    connect(ui->logoutButton, &QPushButton::clicked, this, &MainWindow::onLogoutButtonClicked);
    connect(ui->swapButton, &QPushButton::clicked, this, &MainWindow::onSwapButtonClicked);
    connect(ui->airlineComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onAirlineFilterChanged);
    connect(ui->modifyUsernameButton, &QPushButton::clicked, this, &MainWindow::onModifyUsernameClicked);
    connect(ui->modifyPasswordButton, &QPushButton::clicked, this, &MainWindow::onModifyPasswordClicked);

    // 日期选择连接
    connect(ui->prevWeekButton, &QPushButton::clicked, this, &MainWindow::onPrevWeekClicked);
    connect(ui->nextWeekButton, &QPushButton::clicked, this, &MainWindow::onNextWeekClicked);
    connect(ui->calendarButton, &QPushButton::clicked, this, &MainWindow::onCalendarButtonClicked);
    if (dateButtonGroup) {
        connect(dateButtonGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked),
                this, [this](QAbstractButton *button) {
                    int buttonId = dateButtonGroup->id(button);
                    QDate selected = currentStartDate.addDays(buttonId);

                    if (selected < QDate::currentDate()) {
                        selected = QDate::currentDate();
                    }

                    selectedDate = selected;
                    updateDateButtons();
                    searchFlightsByDate(selected);
                });
    }

    // 导航按钮连接
    connect(ui->bookingNavButton, &QPushButton::clicked, this, &MainWindow::onBookingNavButtonClicked);
    connect(ui->myNavButton, &QPushButton::clicked, this, &MainWindow::onMyNavButtonClicked);

    // "我的"页面按钮连接
    connect(ui->rechargeButton, &QPushButton::clicked, this, &MainWindow::onRechargeButtonClicked);

    if (networkManager) {
        connect(networkManager, &ClientNetworkManager::messageReceived, this, &MainWindow::onMessageReceived);
    }
}

// 查询钱包余额
void MainWindow::queryWalletBalance()
{
    if (!networkManager || !networkManager->isConnected()) {
        ui->balanceLabel->setText("未连接");
        qDebug() << "未连接到服务器，无法查询余额";
        return;
    }

    // 显示查询状态
    ui->balanceLabel->setText("查询中...");

    NetworkMessage msg;
    msg.type = WALLET_QUERY_REQUEST;
    msg.data["username"] = currentUsername;

    networkManager->sendMessage(msg);
    qDebug() << "发送钱包余额查询请求，用户名:" << currentUsername;
}

// 更新钱包显示
void MainWindow::updateWalletDisplay()
{
    QString balanceText = QString("¥%1").arg(userWalletBalance, 0, 'f', 2);
    ui->balanceLabel->setText(balanceText);
    qDebug() << "更新钱包显示:" << balanceText;
}

void MainWindow::onSearchButtonClicked()
{
    searchFlightsByDate(selectedDate);
}

void MainWindow::onAirlineFilterChanged(int index)
{
    Q_UNUSED(index);
    searchFlightsByDate(selectedDate);
}

void MainWindow::onPrevWeekClicked()
{
    QDate newStartDate = currentStartDate.addDays(-7);

    if (newStartDate < QDate::currentDate()) {
        newStartDate = QDate::currentDate();
        ui->prevWeekButton->setEnabled(false);
    } else {
        ui->prevWeekButton->setEnabled(true);
    }

    currentStartDate = newStartDate;
    updateDateButtons();

    selectedDate = currentStartDate;
    updateDateButtons();

    searchFlightsByDate(selectedDate);
}

void MainWindow::onNextWeekClicked()
{
    QDate newStartDate = currentStartDate.addDays(7);

    currentStartDate = newStartDate;
    updateDateButtons();

    ui->prevWeekButton->setEnabled(true);

    selectedDate = currentStartDate;
    updateDateButtons();

    searchFlightsByDate(selectedDate);
}

void MainWindow::onCancelOrderClicked(int orderId, const QString &bookingNumber)
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "确认取消",
                                  QString("确定要取消订单 %1 吗？\n取消后金额将退回钱包。").arg(bookingNumber),
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (!networkManager || !networkManager->isConnected()) {
            QMessageBox::warning(this, "错误", "未连接到服务器");
            return;
        }

        NetworkMessage msg;
        msg.type = ORDER_CANCEL_REQUEST;
        msg.data["order_id"] = orderId;
        msg.data["username"] = currentUsername;

        networkManager->sendMessage(msg);

        QMessageBox::information(this, "处理中",
                                 QString("正在处理订单 %1 的取消请求...").arg(bookingNumber));
    }
}

void MainWindow::showCalendarDialog()
{
    CalendarDialog dialog(this);

    QDate minDate = QDate::currentDate();
    QDate maxDate = QDate::currentDate().addMonths(2);
    maxDate = QDate(maxDate.year(), maxDate.month(), maxDate.daysInMonth());

    dialog.setDateRange(minDate, maxDate);
    dialog.setSelectedDate(selectedDate);

    if (dialog.exec() == QDialog::Accepted) {
        QDate selected = dialog.getSelectedDate();
        if (selected.isValid() && selected >= QDate::currentDate()) {
            selectedDate = selected;

            int daysFromToday = QDate::currentDate().daysTo(selected);
            int weekOffset = daysFromToday / 7 * 7;
            currentStartDate = QDate::currentDate().addDays(weekOffset);

            updateDateButtons();
            ui->prevWeekButton->setEnabled(currentStartDate > QDate::currentDate());
            searchFlightsByDate(selected);
        }
    }
}

void MainWindow::searchFlightsByDate(const QDate &date)
{
    if (!networkManager || !networkManager->isConnected()) {
        QMessageBox::warning(this, "错误", "未连接到服务器");
        return;
    }

    if (ui->departureEdit->text().trimmed().isEmpty() ||
        ui->arrivalEdit->text().trimmed().isEmpty()) {
        return;
    }

    NetworkMessage msg;
    msg.type = FLIGHT_SEARCH_REQUEST;
    msg.data["departure_city"] = ui->departureEdit->text().trimmed();
    msg.data["arrival_city"] = ui->arrivalEdit->text().trimmed();
    msg.data["date"] = date.toString("yyyy-MM-dd");

    // 固定按照出发时间升序排序
    msg.data["sort_by"] = "departure_time";
    msg.data["sort_asc"] = true;

    int airlineIndex = ui->airlineComboBox->currentIndex();
    QString selectedAirline = "";
    switch (airlineIndex) {
    case 1: selectedAirline = "中国国航"; break;
    case 2: selectedAirline = "东方航空"; break;
    case 3: selectedAirline = "南方航空"; break;
    case 4: selectedAirline = "海南航空"; break;
    case 5: selectedAirline = "厦门航空"; break;
    default: selectedAirline = "";
    }
    msg.data["airline"] = selectedAirline;

    networkManager->sendMessage(msg);
    ui->flightListWidget->clear();
    ui->flightListWidget->addItem("正在搜索" + date.toString("yyyy年MM月dd日") + "的航班...");
}

void MainWindow::searchFlights()
{
    searchFlightsByDate(selectedDate);
}

void MainWindow::onFlightItemDoubleClicked(QListWidgetItem *item)
{
    Q_UNUSED(item);
    // 实现将在FlightItemWidget中处理
}

void MainWindow::showFlightDetail(const Flight &flight)
{
    FlightDetailDialog *dialog = new FlightDetailDialog(flight, networkManager,
                                                        currentUsername, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->exec();
}

void MainWindow::onLogoutButtonClicked()
{
    if (networkManager && networkManager->isConnected()) {
        networkManager->disconnectFromServer();
    }
    this->close();
    emit logoutRequested();
}

void MainWindow::onSwapButtonClicked()
{
    QString departure = ui->departureEdit->text();
    QString arrival = ui->arrivalEdit->text();
    ui->departureEdit->setText(arrival);
    ui->arrivalEdit->setText(departure);
}

void MainWindow::onMessageReceived(const NetworkMessage &message)
{
    switch (message.type) {
    case FLIGHT_SEARCH_RESPONSE:
    {
        bool success = message.data["success"].toBool();
        if (success) {
            QJsonArray flightsArray = message.data["flights"].toArray();
            QList<Flight> flights;
            for (auto it = flightsArray.constBegin(); it != flightsArray.constEnd(); ++it) {
                Flight flight = Flight::fromJson((*it).toObject());
                flights.append(flight);
            }
            currentFlights = flights;
            displayFlights(flights);

            if (!flights.isEmpty()) {
                ui->flightListWidget->insertItem(0,
                                                 "📅 搜索日期: " + selectedDate.toString("yyyy年MM月dd日"));
            }
        } else {
            QMessageBox::warning(this, "搜索失败", message.data["message"].toString());
            ui->flightListWidget->clear();
            ui->flightListWidget->addItem("搜索"
                                          + selectedDate.toString("yyyy年MM月dd日") + "的航班失败");
        }
        break;
    }

    case ORDER_LIST_RESPONSE:
    {
        bool ok = message.data["success"].toBool();
        if (ok) {
            displayOrders(message.data["orders"].toArray());
        } else {
            QMessageBox::warning(this, "获取订单失败",
                                 message.data["message"].toString());
        }
        break;
    }

    case ORDER_CANCEL_RESPONSE:
    {
        bool success = message.data["success"].toBool();
        QString resultMsg = message.data["message"].toString();

        if (success) {
            double refundAmount = message.data["refund_amount"].toDouble();
            double newBalance = message.data["new_balance"].toDouble();
            QString bookingNumber = message.data["booking_number"].toString();

            QString successMsg = QString(
                                     "✅ 订单取消成功！\n\n"
                                     "订单号：%1\n"
                                     "退回金额：¥%2\n"
                                     "当前余额：¥%3\n\n"
                                     "座位已释放，金额已退回钱包。"
                                     ).arg(bookingNumber)
                                     .arg(refundAmount, 0, 'f', 2)
                                     .arg(newBalance, 0, 'f', 2);

            QMessageBox::information(this, "取消成功", successMsg);

            loadOrders();

            // 新增：订单取消后更新钱包余额
            userWalletBalance = newBalance;
            updateWalletDisplay();

        } else {
            QMessageBox::warning(this, "取消失败", resultMsg);
        }
        break;
    }

    case WALLET_QUERY_RESPONSE:
    {
        bool success = message.data["success"].toBool();
        if (success) {
            userWalletBalance = message.data["balance"].toDouble();
            walletBalanceChecked = true;
            updateWalletDisplay();
            qDebug() << "主窗口收到钱包余额查询成功:" << userWalletBalance;

            // 关键修复: 如果钱包对话框处于打开状态，也需要通知它
            // 查找是否有打开的WalletDialog并更新其显示
            QList<WalletDialog*> dialogs = this->findChildren<WalletDialog*>();
            for (WalletDialog* dialog : dialogs) {
                qDebug() << "找到打开的WalletDialog，转发消息";
                // 调用对话框的消息处理方法
                dialog->onMessageReceived(message);
            }
        } else {
            QString errorMsg = message.data["message"].toString();
            ui->balanceLabel->setText("查询失败");
            qDebug() << "钱包余额查询失败:" << errorMsg;
        }
        break;
    }

    case CHANGE_USERNAME_RESPONSE:
    {
        bool success = message.data["success"].toBool();
        QString resultMsg = message.data["message"].toString();
        QString newUsername = message.data["new_username"].toString();

        if (success) {
            qDebug() << "服务器确认用户名修改成功:" << newUsername;
        } else {
            qDebug() << "服务器返回用户名修改失败:" << resultMsg;
        }
        break;
    }

    case CHANGE_PASSWORD_RESPONSE:
    {
        bool success = message.data["success"].toBool();
        QString resultMsg = message.data["message"].toString();

        if (success) {
            qDebug() << "密码修改成功";
        } else {
            qDebug() << "密码修改失败:" << resultMsg;
        }
        break;
    }

    default:
        break;
    }
}

void MainWindow::displayFlights(const QList<Flight> &flights)
{
    ui->flightListWidget->clear();

    if (flights.isEmpty()) {
        QString departure = ui->departureEdit->text().trimmed();
        QString arrival = ui->arrivalEdit->text().trimmed();

        QString airline = "";
        int airlineIndex = ui->airlineComboBox->currentIndex();
        switch (airlineIndex) {
        case 1: airline = "中国国航"; break;
        case 2: airline = "东方航空"; break;
        case 3: airline = "南方航空"; break;
        case 4: airline = "海南航空"; break;
        case 5: airline = "厦门航空"; break;
        default: airline = "所有航空公司"; break;
        }

        QString dateStr = selectedDate.toString("yyyy年MM月dd日");

        QString message = QString(
                              "<div style='text-align: center; padding: 20px; font-family: Microsoft YaHei;'>"
                              "<p style='font-size: 16px; color: #333; margin-bottom: 10px;'>您搜索的 "
                              "<span style='color: #1e88e5; font-weight: bold; font-size: 18px;'>%1</span> "
                              "到 "
                              "<span style='color: #1e88e5; font-weight: bold; font-size: 18px;'>%2</span> "
                              "的 "
                              "<span style='color: #ff9800; font-weight: bold; font-size: 18px;'>%3</span> "
                              "航班，</p>"
                              "<p style='font-size: 16px; color: #333; margin-bottom: 10px;'>"
                              "在 <span style='color: #f44336; font-weight: bold; font-size: 18px;'>%4</span> "
                              "无直飞航班，</p>"
                              "<p style='font-size: 14px; color: #666; margin-top: 15px;'>"
                              "💡 可以尝试更换其他日期查看。</p>"
                              "</div>")
                              .arg(departure, arrival, airline, dateStr);

        QWidget *widget = new QWidget();
        widget->setAttribute(Qt::WA_TransparentForMouseEvents);
        widget->setEnabled(false);

        QVBoxLayout *layout = new QVBoxLayout(widget);
        layout->setContentsMargins(10, 20, 10, 20);

        QLabel *label = new QLabel();
        label->setText(message);
        label->setAlignment(Qt::AlignCenter);
        label->setWordWrap(true);
        label->setStyleSheet(
            "QLabel {"
            "    background: transparent;"
            "    border: none;"
            "}"
            );

        layout->addWidget(label);

        QListWidgetItem *item = new QListWidgetItem();
        item->setSizeHint(QSize(ui->flightListWidget->width() - 20, 180));
        item->setFlags(item->flags() & ~Qt::ItemIsSelectable & ~Qt::ItemIsEnabled);
        item->setBackground(QBrush(Qt::transparent));
        item->setData(Qt::UserRole, "no_flights_item");

        ui->flightListWidget->addItem(item);
        ui->flightListWidget->setItemWidget(item, widget);

        ui->flightListWidget->setSelectionMode(QAbstractItemView::NoSelection);
        return;
    }

    ui->flightListWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    for (const Flight &flight : flights) {
        FlightItemWidget *widget = new FlightItemWidget(flight);
        connect(widget, &FlightItemWidget::bookRequested, this, &MainWindow::showFlightDetail);

        QListWidgetItem *item = new QListWidgetItem();
        item->setSizeHint(widget->sizeHint());

        ui->flightListWidget->addItem(item);
        ui->flightListWidget->setItemWidget(item, widget);
    }
}

void MainWindow::addFlightItem(const Flight &flight)
{
    FlightItemWidget *widget = new FlightItemWidget(flight);
    connect(widget, &FlightItemWidget::bookRequested, this, &MainWindow::showFlightDetail);

    QListWidgetItem *item = new QListWidgetItem();
    item->setSizeHint(widget->sizeHint());

    ui->flightListWidget->addItem(item);
    ui->flightListWidget->setItemWidget(item, widget);
}

void MainWindow::setupNavigation()
{
    navButtonGroup = new QButtonGroup(this);
    navButtonGroup->setExclusive(true);

    navButtonGroup->addButton(ui->bookingNavButton, 0);
    navButtonGroup->addButton(ui->myNavButton, 1);

    ui->bookingNavButton->setChecked(true);
    ui->stackedWidget->setCurrentWidget(ui->bookingPage);

    updateNavButtonStyles();
}

void MainWindow::updateNavButtonStyles()
{
    if (ui->bookingNavButton->isChecked()) {
        ui->bookingNavButton->setStyleSheet(
            "QPushButton {"
            "    background: #1565c0;"
            "    color: white;"
            "    border: none;"
            "    border-radius: 0px;"
            "    padding: 20px 0px;"
            "    font-size: 16px;"
            "    font-weight: bold;"
            "    min-width: 200px;"
            "    border-top: 3px solid #ff9800;"
            "}"
            "QPushButton:hover {"
            "    background: #1976d2;"
            "}"
            );

        ui->myNavButton->setStyleSheet(
            "QPushButton {"
            "    background: #f5f5f5;"
            "    color: #666;"
            "    border: none;"
            "    border-radius: 0px;"
            "    padding: 20px 0px;"
            "    font-size: 16px;"
            "    font-weight: bold;"
            "    min-width: 200px;"
            "}"
            "QPushButton:hover {"
            "    background: #e0e0e0;"
            "}"
            "QPushButton:checked {"
            "    background: #ffffff;"
            "    color: #1e88e5;"
            "    border-top: 3px solid #1e88e5;"
            "}"
            );
    } else {
        ui->myNavButton->setStyleSheet(
            "QPushButton {"
            "    background: #1565c0;"
            "    color: white;"
            "    border: none;"
            "    border-radius: 0px;"
            "    padding: 20px 0px;"
            "    font-size: 16px;"
            "    font-weight: bold;"
            "    min-width: 200px;"
            "    border-top: 3px solid #ff9800;"
            "}"
            "QPushButton:hover {"
            "    background: #1976d2;"
            "}"
            );

        ui->bookingNavButton->setStyleSheet(
            "QPushButton {"
            "    background: #f5f5f5;"
            "    color: #666;"
            "    border: none;"
            "    border-radius: 0px;"
            "    padding: 20px 0px;"
            "    font-size: 16px;"
            "    font-weight: bold;"
            "    min-width: 200px;"
            "}"
            "QPushButton:hover {"
            "    background: #e0e0e0;"
            "}"
            "QPushButton:checked {"
            "    background: #ffffff;"
            "    color: #1e88e5;"
            "    border-top: 3px solid #1e88e5;"
            "}"
            );
    }
}

void MainWindow::onBookingNavButtonClicked()
{
    ui->stackedWidget->setCurrentWidget(ui->bookingPage);
    updateNavButtonStyles();
}

void MainWindow::onMyNavButtonClicked()
{
    ui->stackedWidget->setCurrentWidget(ui->myPage);
    updateNavButtonStyles();

    loadUserInfo();
    loadOrders();

    // 修复: 确保每次切换时都查询余额
    qDebug() << "切换到'我的'页面，查询余额...";

    // 先更新界面显示状态
    ui->balanceLabel->setText("查询中...");

    // 延迟一小段时间确保UI更新完成
    QTimer::singleShot(100, this, [this]() {
        queryWalletBalance();
    });
}

void MainWindow::loadUserInfo()
{
    if (!currentUsername.isEmpty()) {
        QString firstChar = currentUsername.left(1).toUpper();
        ui->userAvatarLabel->setText(firstChar);
    }
}

void MainWindow::loadOrders()
{
    ui->ordersListWidget->clear();
    ui->ordersListWidget->addItem("正在加载订单...");

    if (!networkManager || !networkManager->isConnected()) {
        ui->ordersListWidget->clear();
        ui->ordersListWidget->addItem("未连接到服务器");
        return;
    }

    NetworkMessage msg;
    msg.type = ORDER_LIST_REQUEST;
    msg.data["username"] = currentUsername;

    networkManager->sendMessage(msg);
    qDebug() << "自动发送订单列表请求，用户名:" << currentUsername;
}

void MainWindow::onRechargeButtonClicked()
{
    WalletDialog *dialog = new WalletDialog(currentUsername, networkManager, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);

    // 新增：连接余额更新信号
    connect(dialog, &WalletDialog::balanceUpdated, this, [this]() {
        qDebug() << "收到余额更新信号，重新查询余额";
        // 延迟一小段时间，确保服务器已经处理完充值请求
        QTimer::singleShot(500, this, [this]() {
            queryWalletBalance();
        });
    });

    dialog->exec();
}

void MainWindow::onViewAllOrdersButtonClicked()
{
    if (!networkManager || !networkManager->isConnected()) {
        QMessageBox::warning(this, "错误", "未连接到服务器");
        return;
    }

    NetworkMessage msg;
    msg.type = ORDER_LIST_REQUEST;
    msg.data["username"] = currentUsername;

    networkManager->sendMessage(msg);
}

void MainWindow::displayOrders(const QJsonArray &orders)
{
    ui->ordersListWidget->clear();

    if (orders.isEmpty()) {
        ui->ordersListWidget->addItem("暂无订单");
        return;
    }

    for (int i = 0; i < orders.size(); i++) {
        QJsonObject obj = orders[i].toObject();
        int orderId = obj["order_id"].toInt();
        QString bookingNumber = obj["booking_number"].toString();
        QString flightInfo = obj["flight_info"].toString();
        QString date = obj["date"].toString();
        double price = obj["price"].toDouble();
        int status = obj["status"].toInt();

        QWidget *orderWidget = new QWidget();
        orderWidget->setObjectName(QString("orderWidget_%1").arg(orderId));
        orderWidget->setStyleSheet("background: white; border: none;");
        orderWidget->setMinimumHeight(40);
        orderWidget->setMaximumHeight(45);

        QHBoxLayout *layout = new QHBoxLayout(orderWidget);
        layout->setContentsMargins(8, 4, 8, 4);
        layout->setSpacing(8);

        QString statusStr;
        QString statusIcon;
        QColor statusColor;

        switch (status) {
        case 1:
            statusStr = "已预订";
            statusIcon = "✅";
            statusColor = QColor(0, 128, 0);
            break;
        case 2:
            statusStr = "已取消";
            statusIcon = "❌";
            statusColor = QColor(128, 0, 0);
            break;
        default:
            statusStr = "未知";
            statusIcon = "❓";
            statusColor = Qt::darkGray;
        }

        QLabel *infoLabel = new QLabel();
        infoLabel->setText(QString("<span style='font-size: 10px;'>%1 %2</span> | "
                                   "<span style='color: #1e88e5; font-size: 10px;'>%3</span> | "
                                   "<span style='color: #666; font-size: 10px;'>%4</span> | "
                                   "<span style='color: #ff5722; font-size: 10px;'>¥%5</span> | "
                                   "<span style='color: %6; font-size: 10px;'>%7</span>")
                               .arg(statusIcon, bookingNumber, flightInfo, date)
                               .arg(price, 0, 'f', 0)
                               .arg(statusColor.name(), statusStr));

        infoLabel->setStyleSheet("QLabel {"
                                 "    background: transparent;"
                                 "    color: #333;"
                                 "    font-size: 10px;"
                                 "    padding: 1px;"
                                 "    margin: 0;"
                                 "}");
        infoLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

        layout->addWidget(infoLabel, 1);

        if (status == 1) {
            QPushButton *cancelButton = new QPushButton("取消订单");
            cancelButton->setObjectName(QString("cancelBtn_%1").arg(orderId));
            cancelButton->setFixedSize(75, 26);

            cancelButton->setStyleSheet(
                "QPushButton {"
                "    background: #f44336;"
                "    color: white;"
                "    border: none;"
                "    border-radius: 3px;"
                "    font-size: 10px;"
                "    font-weight: bold;"
                "    padding: 3px 6px;"
                "    margin: 0;"
                "}"
                "QPushButton:hover {"
                "    background: #d32f2f;"
                "}"
                "QPushButton:pressed {"
                "    background: #b71c1c;"
                "}"
                );

            connect(cancelButton, &QPushButton::clicked, this, [this, orderId, bookingNumber]() {
                onCancelOrderClicked(orderId, bookingNumber);
            });

            layout->addWidget(cancelButton);

        } else {
            QLabel *statusLabel = new QLabel(statusStr);
            statusLabel->setStyleSheet(QString(
                                           "QLabel {"
                                           "    color: %1;"
                                           "    font-size: 10px;"
                                           "    font-weight: bold;"
                                           "    background: transparent;"
                                           "    padding: 3px 8px;"
                                           "    margin: 0;"
                                           "}"
                                           ).arg(statusColor.name()));
            statusLabel->setAlignment(Qt::AlignCenter);
            statusLabel->setFixedSize(75, 26);
            layout->addWidget(statusLabel);
        }

        QListWidgetItem *item = new QListWidgetItem();
        item->setSizeHint(QSize(ui->ordersListWidget->width() - 20, 42));
        item->setData(Qt::UserRole, orderId);
        item->setData(Qt::UserRole + 1, bookingNumber);
        item->setBackground(Qt::white);

        ui->ordersListWidget->addItem(item);
        ui->ordersListWidget->setItemWidget(item, orderWidget);
    }
}

void MainWindow::onDateButtonClicked()
{
    // 功能已经在lambda表达式中实现
}

void MainWindow::onCalendarButtonClicked()
{
    showCalendarDialog();
}

void MainWindow::onModifyUsernameClicked()
{
    qDebug() << "=== 修改用户名按钮被点击 ===";

    if (!networkManager || !networkManager->isConnected()) {
        QMessageBox::warning(this, "连接错误", "未连接到服务器");
        return;
    }

    QString displayedName = ui->userNameLabel->text();
    QString currentUser = displayedName;

    if (displayedName.startsWith("欢迎，")) {
        currentUser = displayedName.mid(3);
    }

    if (currentUser.isEmpty()) {
        QMessageBox::warning(this, "错误", "无法获取当前用户名");
        return;
    }

    qDebug() << "当前用户名:" << currentUser;

    ChangeUsernameDialog *dialog = new ChangeUsernameDialog(networkManager,
                                                            currentUser,
                                                            this);

    connect(dialog, &ChangeUsernameDialog::usernameChanged,
            this, &MainWindow::onUsernameChanged);

    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->exec();
}

void MainWindow::onModifyPasswordClicked()
{
    qDebug() << "=== 修改密码按钮被点击 ===";

    if (!networkManager || !networkManager->isConnected()) {
        QMessageBox::warning(this, "连接错误", "未连接到服务器");
        return;
    }

    QString displayedName = ui->userNameLabel->text();
    QString currentUser = displayedName;

    if (displayedName.startsWith("欢迎，")) {
        currentUser = displayedName.mid(3);
    }

    if (currentUser.isEmpty()) {
        QMessageBox::warning(this, "错误", "无法获取当前用户名");
        return;
    }

    qDebug() << "当前用户名:" << currentUser;

    ChangePasswordDialog *dialog = new ChangePasswordDialog(networkManager,
                                                            currentUser,
                                                            this);

    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->exec();
}

void MainWindow::onUsernameChanged(const QString& newUsername)
{
    qDebug() << "用户名修改成功，新用户名:" << newUsername;

    currentUsername = newUsername;

    if (ui->userNameLabel) {
        QString displayName = "欢迎，" + newUsername;
        ui->userNameLabel->setText(displayName);

        if (ui->userAvatarLabel) {
            if (!newUsername.isEmpty()) {
                QString firstLetter = newUsername.left(1).toUpper();
                ui->userAvatarLabel->setText(firstLetter);
            }
        }
    }

    // 新增：用户名修改后重新查询余额
    queryWalletBalance();
}

// 最后包含moc文件
#include "mainwindow.moc"
