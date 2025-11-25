#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "../Common/protocol.h"
#include <QListWidgetItem>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QDebug>
#include <QDate>
#include <QTimer>           // 添加QTimer头文件
#include <QJsonArray>       // 添加QJsonArray头文件
#include <QJsonDocument>    // 添加QJsonDocument头文件

// 自定义航班列表项Widget
class FlightItemWidget : public QWidget
{
    Q_OBJECT

public:
    FlightItemWidget(const Flight &flight, QWidget *parent = nullptr)
        : QWidget(parent), flight(flight)
    {
        setupUI();  // 直接调用，不需要类名限定
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
    void setupUI();  // 声明，不要在这里实现

private:
    Flight flight;
};

// 在类外部实现 setupUI 方法
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

    QLabel *priceLabel = new QLabel(QString("¥%1").arg(flight.getPrice()));
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

    // 设置日期为2025年11月24日
    ui->dateEdit->setDate(QDate(2025, 11, 24));

    // 设置默认城市（可选）
    ui->departureEdit->setText("广州");
    ui->arrivalEdit->setText("宜宾");

    qDebug() << "主窗口初始化完成";

    setupConnections();

    // 延迟搜索，确保界面完全加载
    QTimer::singleShot(500, this, &MainWindow::onSearchButtonClicked);
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

    if (networkManager) {
        connect(networkManager, &ClientNetworkManager::messageReceived, this, &MainWindow::onMessageReceived);
    }
}

void MainWindow::onSearchButtonClicked()
{
    searchFlights();
}

void MainWindow::onSortChanged(int index)
{
    // 重新排序当前航班列表
    if (!currentFlights.isEmpty()) {
        // 这里可以实现本地排序，或者重新请求服务器排序
        searchFlights();
    }
}

void MainWindow::onFlightItemDoubleClicked(QListWidgetItem *item)
{
    auto *widget = qobject_cast<FlightItemWidget*>(ui->flightListWidget->itemWidget(item));
    if (widget) {
        showFlightDetail(widget->getFlight());
    }
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

void MainWindow::searchFlights()
{
    if (!networkManager || !networkManager->isConnected()) {
        QMessageBox::warning(this, "错误", "未连接到服务器");
        return;
    }

    NetworkMessage msg;
    msg.type = FLIGHT_SEARCH_REQUEST;
    msg.data["departure_city"] = ui->departureEdit->text().trimmed();
    msg.data["arrival_city"] = ui->arrivalEdit->text().trimmed();
    msg.data["date"] = ui->dateEdit->date().toString("yyyy-MM-dd");

    // 设置排序
    int sortIndex = ui->sortComboBox->currentIndex();
    switch (sortIndex) {
    case 0: // 出发时间
        msg.data["sort_by"] = "departure_time";
        msg.data["sort_asc"] = true;
        break;
    case 1: // 价格从低到高
        msg.data["sort_by"] = "price";
        msg.data["sort_asc"] = true;
        break;
    case 2: // 价格从高到低
        msg.data["sort_by"] = "price";
        msg.data["sort_asc"] = false;
        break;
    case 3: // 飞行时长
        msg.data["sort_by"] = "duration";
        msg.data["sort_asc"] = true;
        break;
    }

    networkManager->sendMessage(msg);
    ui->flightListWidget->clear();
    ui->flightListWidget->addItem("正在搜索航班...");
}

void MainWindow::onMessageReceived(const NetworkMessage &message)
{
    if (message.type == FLIGHT_SEARCH_RESPONSE) {
        bool success = message.data["success"].toBool();

        if (success) {
            QJsonArray flightsArray = message.data["flights"].toArray();
            QList<Flight> flights;

            for (const QJsonValue &value : flightsArray) {
                Flight flight = Flight::fromJson(value.toObject());
                flights.append(flight);
            }

            currentFlights = flights;
            displayFlights(flights);
        } else {
            QMessageBox::warning(this, "搜索失败", message.data["message"].toString());
            ui->flightListWidget->clear();
            ui->flightListWidget->addItem("搜索失败");
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

void MainWindow::showFlightDetail(const Flight &flight)
{
    // 显示航班详情对话框
    QMessageBox::information(this,
                             QString("航班 %1 详情").arg(flight.getFlightNumber()),
                             QString("航班号: %1\n"
                                     "航空公司: %2\n"
                                     "航线: %3 → %4\n"
                                     "时间: %5 - %6\n"
                                     "时长: %7\n"
                                     "机型: %8\n"
                                     "价格: ¥%9\n"
                                     "剩余座位: %10")
                                 .arg(flight.getFlightNumber())
                                 .arg(flight.getAirline())
                                 .arg(flight.getDepartureCity())
                                 .arg(flight.getArrivalCity())
                                 .arg(flight.getDepartureTime().toString("MM月dd日 hh:mm"))
                                 .arg(flight.getArrivalTime().toString("MM月dd日 hh:mm"))
                                 .arg(flight.getDurationString())
                                 .arg(flight.getAircraftType())
                                 .arg(flight.getPrice())
                                 .arg(flight.getAvailableSeats())
                             );
}

#include "mainwindow.moc"
