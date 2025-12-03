#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "../Common/protocol.h"
#include "calendardialog.h"
#include "flightdetaildialog.h"  // 新增头文件
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

// 自定义航班列表项Widget
class FlightItemWidget : public QWidget
{
    Q_OBJECT

public:
    FlightItemWidget(const Flight &flight, QWidget *parent = nullptr)
        : QWidget(parent), flight(flight)
    {
        setupUI();
    }

    Flight getFlight() const { return flight; }

signals:
    void bookRequested(const Flight &flight);

private slots:
    void onBookButtonClicked()
    {
        emit bookRequested(flight);
    }

private:
    void setupUI();

private:
    Flight flight;
};

void FlightItemWidget::setupUI()
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

    QLabel *aircraftLabel = new QLabel(flight.getAircraftType());
    aircraftLabel->setStyleSheet("font-size: 12px; color: #999;");

    headerLayout->addWidget(flightNumberLabel);
    headerLayout->addWidget(airlineLabel);
    headerLayout->addWidget(aircraftLabel);
    headerLayout->addStretch();

    // 时间和城市信息
    QHBoxLayout *timeLayout = new QHBoxLayout();

    // 出发信息
    QVBoxLayout *departureLayout = new QVBoxLayout();
    QLabel *departureTimeLabel = new QLabel(flight.getDepartureTimeString());
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
    QLabel *arrivalTimeLabel = new QLabel(flight.getArrivalTimeString());
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

    QLabel *priceLabel = new QLabel(QString("¥%1").arg(flight.getPrice(), 0, 'f', 2));
    priceLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #ff5722;");

    QLabel *seatsLabel = new QLabel(QString("剩余%1张").arg(flight.getAvailableSeats()));
    seatsLabel->setStyleSheet("font-size: 12px; color: #4caf50; padding: 2px 0px;");

    QPushButton *bookButton = new QPushButton("立即预订");
    bookButton->setStyleSheet(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #ff5722, stop:1 #e64a19);"
        "    color: white;"
        "    border: none;"
        "    border-radius: 6px;"
        "    padding: 10px 20px;"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "    min-width: 90px;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #e64a19, stop:1 #d84315);"
        "}"
        "QPushButton:pressed {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #d84315, stop:1 #c62828);"
        "}"
        );

    connect(bookButton, &QPushButton::clicked, this, &FlightItemWidget::onBookButtonClicked);

    priceLayout->addWidget(priceLabel);
    priceLayout->addWidget(seatsLabel);
    priceLayout->addWidget(bookButton);

    layout->addLayout(infoLayout, 1);
    layout->addLayout(priceLayout);

    // 使用简单的边框样式替代阴影
    setStyleSheet(
        "FlightItemWidget {"
        "    background: white;"
        "    border: 1px solid #e0e0e0;"
        "    border-radius: 8px;"
        "    margin: 5px 15px;"
        "}"
        "FlightItemWidget:hover {"
        "    background: #f8fdff;"
        "    border-color: #bbdefb;"
        "}"
        );
}

MainWindow::MainWindow(const QString &username, ClientNetworkManager* networkManager, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), currentUsername(username), networkManager(networkManager)
{
    ui->setupUi(this);

    // 设置用户信息
    ui->userLabel->setText(QString("欢迎，%1").arg(username));

    // 设置默认城市
    ui->departureEdit->setText("广州");
    ui->arrivalEdit->setText("宜宾");

    qDebug() << "主窗口初始化完成";

    // 初始化日期选择系统
    setupDateSelection();

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

        QString buttonText = QString("%1\n%2").arg(dayName).arg(buttonDate.toString("MM/dd"));
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
    if (ui) {
        delete ui;
    }
}

void MainWindow::setupConnections()
{
    connect(ui->searchButton, &QPushButton::clicked, this, &MainWindow::onSearchButtonClicked);
    connect(ui->sortComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onSortChanged);
    connect(ui->flightListWidget, &QListWidget::itemDoubleClicked, this, &MainWindow::onFlightItemDoubleClicked);
    connect(ui->logoutButton, &QPushButton::clicked, this, &MainWindow::onLogoutButtonClicked);
    connect(ui->swapButton, &QPushButton::clicked, this, &MainWindow::onSwapButtonClicked);
    connect(ui->airlineComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onAirlineFilterChanged);

    // 日期选择连接
    connect(ui->prevWeekButton, &QPushButton::clicked, this, &MainWindow::onPrevWeekClicked);
    connect(ui->nextWeekButton, &QPushButton::clicked, this, &MainWindow::onNextWeekClicked);
    connect(ui->calendarButton, &QPushButton::clicked, this, &MainWindow::onCalendarButtonClicked);
    connect(dateButtonGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked),
            this, [this](QAbstractButton *button) {
                int buttonId = dateButtonGroup->id(button);
                QDate selected = currentStartDate.addDays(buttonId);

                // 确保不选择过去日期
                if (selected < QDate::currentDate()) {
                    selected = QDate::currentDate();
                }

                selectedDate = selected;
                updateDateButtons();
                searchFlightsByDate(selected);
            });

    if (networkManager) {
        connect(networkManager, &ClientNetworkManager::messageReceived, this, &MainWindow::onMessageReceived);
    }
}

void MainWindow::onSearchButtonClicked()
{
    searchFlightsByDate(selectedDate);
}

void MainWindow::onSortChanged(int index)
{
    Q_UNUSED(index)
    searchFlightsByDate(selectedDate);
}

void MainWindow::onAirlineFilterChanged(int index)
{
    qDebug() << "航空公司筛选改变，索引:" << index;
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

void MainWindow::onCalendarButtonClicked()
{
    showCalendarDialog();
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

    int sortIndex = ui->sortComboBox->currentIndex();
    switch (sortIndex) {
    case 0: msg.data["sort_by"] = "departure_time"; msg.data["sort_asc"] = true; break;
    case 1: msg.data["sort_by"] = "price"; msg.data["sort_asc"] = true; break;
    case 2: msg.data["sort_by"] = "price"; msg.data["sort_asc"] = false; break;
    case 3: msg.data["sort_by"] = "duration"; msg.data["sort_asc"] = true; break;
    }

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

    qDebug() << "发送搜索请求 - 日期:" << date.toString("yyyy-MM-dd")
             << "出发:" << ui->departureEdit->text()
             << "到达:" << ui->arrivalEdit->text()
             << "航空公司:" << (selectedAirline.isEmpty() ? "所有" : selectedAirline);

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
    auto *widget = qobject_cast<FlightItemWidget*>(ui->flightListWidget->itemWidget(item));
    if (widget) {
        showFlightDetail(widget->getFlight());
    }
}

// 修改：显示航班详情对话框
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
    if (message.type == FLIGHT_SEARCH_RESPONSE) {
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
                ui->flightListWidget->insertItem(0, "📅 搜索日期: " + selectedDate.toString("yyyy年MM月dd日"));
            }
        } else {
            QMessageBox::warning(this, "搜索失败", message.data["message"].toString());
            ui->flightListWidget->clear();
            ui->flightListWidget->addItem("搜索" + selectedDate.toString("yyyy年MM月dd日") + "的航班失败");
        }
    }
}

void MainWindow::displayFlights(const QList<Flight> &flights)
{
    ui->flightListWidget->clear();

    if (flights.isEmpty()) {
        ui->flightListWidget->addItem("未找到符合条件的航班");
        return;
    }

    for (const Flight &flight : flights) {
        addFlightItem(flight);
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

void MainWindow::onDateButtonClicked()
{
    // 功能已经在lambda表达式中实现
}

#include "mainwindow.moc"
