#include "testclient.h"
#include "ui_testclient.h"
#include "clientnetworkmanager.h"
#include "registerdialog.h"
#include "mainwindow.h"
#include "../Common/protocol.h"
#include <QJsonDocument>
#include <QDebug>
#include <QMessageBox>
#include <QTimer>
#include <QCloseEvent>
#include <QFile>
#include <QFileInfo>
#include <QPixmap>
#include <QPalette>
#include <QApplication>

class PlaceholderFilter : public QObject
{
public:
    PlaceholderFilter(QLineEdit *edit, const QString &text)
        : QObject(edit), lineEdit(edit), placeholder(text) {}

protected:
    bool eventFilter(QObject *obj, QEvent *event) override {
        if (obj == lineEdit) {
            if (event->type() == QEvent::FocusIn) {
                lineEdit->setPlaceholderText("");
            } else if (event->type() == QEvent::FocusOut) {
                if (lineEdit->text().isEmpty())
                    lineEdit->setPlaceholderText(placeholder);
            }
        }
        return false;
    }

private:
    QLineEdit *lineEdit;
    QString placeholder;
};

void TestClient::showLoginWindow()
{
    this->show();
    this->raise();
    this->activateWindow();
}

TestClient::TestClient(QWidget *parent)
    : QWidget(parent),
    ui(new Ui::TestClient),
    networkManager(new ClientNetworkManager(this)),
    registerDialog(nullptr),
    mainWindow(nullptr)
{
    ui->setupUi(this);
    this->setFixedSize(900, 500);

    // 设置背景
    setBackgroundImage();

    ui->outputLabel->hide();
    ui->textEditOutput->hide();

    // 清除可能的初始文本
    ui->usernameEdit->clear();
    ui->passwordEdit->clear();

    // 设置占位符文本
    ui->usernameEdit->setPlaceholderText("请输入用户名");
    ui->usernameEdit->installEventFilter(new PlaceholderFilter(ui->usernameEdit, "请输入用户名"));

    ui->passwordEdit->setPlaceholderText("请输入密码");
    ui->passwordEdit->installEventFilter(new PlaceholderFilter(ui->passwordEdit, "请输入密码"));

    // 连接注册链接按钮的信号
    connect(ui->registerLinkButton, &QPushButton::clicked, this, &TestClient::on_registerLinkButton_clicked);

    setupConnections();

    // 延迟设置图标，确保UI完全初始化
    QTimer::singleShot(100, this, &TestClient::setupIcons);

    QTimer::singleShot(100, this, [this]() {
        autoConnect();
    });
}

void TestClient::setBackgroundImage()
{
    // 使用完整路径
    QString backgroundPath = "C:/FlightBookingSystem/Client/login-background.png";
    QPixmap background(backgroundPath);

    if (!background.isNull()) {
        // 设置整个窗口的背景
        QPixmap scaledBackground = background.scaled(this->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        QPalette palette;
        palette.setBrush(QPalette::Window, QBrush(scaledBackground));
        this->setPalette(palette);
        this->setAutoFillBackground(true);

        // 设置左侧登录框为完全透明
        ui->loginFrame->setStyleSheet(
            "QFrame {"
            "    background: transparent;"
            "    border: none;"
            "    border-right: 2px solid rgba(255, 255, 255, 0.3);"
            "    border-radius: 0px;"
            "}"
            );

        qDebug() << "背景图片加载成功:" << backgroundPath;
    } else {
        qDebug() << "背景图片加载失败，路径:" << backgroundPath;
        // 使用黄蓝渐变作为fallback
        this->setStyleSheet(
            "QWidget {"
            "    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #FFD700, stop:0.5 #87CEEB, stop:1 #1E90FF);"
            "}"
            );
    }
}

void TestClient::setupIcons()
{
    // 使用默认图标（表情符号）
    ui->userIconLabel->setText("👤");
    ui->userIconLabel->setStyleSheet("font-size: 24px; color: #FFD700;");  // 黄色图标

    ui->pwdIconLabel->setText("🔒");
    ui->pwdIconLabel->setStyleSheet("font-size: 24px; color: #FFD700;");  // 黄色图标

    // 设置右侧插图 - 使用UI中设置的固定尺寸
    QString loginPath = "C:/FlightBookingSystem/Client/login.jpg";
    QPixmap loginPixmap(loginPath);

    if (!loginPixmap.isNull()) {
        // 直接使用UI中设置的400x500尺寸
        QSize targetSize(400, 500);

        // 使用高质量缩放填满整个区域
        QPixmap scaledPixmap = loginPixmap.scaled(targetSize,
                                                  Qt::IgnoreAspectRatio,
                                                  Qt::SmoothTransformation);

        ui->loginIllustrationLabel->setPixmap(scaledPixmap);
        ui->loginIllustrationLabel->setScaledContents(true);  // 启用自动缩放以确保填满

        qDebug() << "右侧插图加载成功 - 原始尺寸:" << loginPixmap.size() << "目标尺寸:" << targetSize;
    } else {
        // 备用方案
        ui->loginIllustrationLabel->setText("✈️");
        ui->loginIllustrationLabel->setStyleSheet(
            "QLabel {"
            "    font-size: 150px;"
            "    color: white;"
            "    background: transparent;"
            "}"
            );
        qDebug() << "右侧插图加载失败，路径:" << loginPath;
    }
}

TestClient::~TestClient()
{
    if (networkManager->isConnected()) {
        ui->textEditOutput->append("自动断开服务器连接...");
        networkManager->disconnectFromServer();
    }

    delete registerDialog;
    delete mainWindow;
    delete ui;
}

void TestClient::showMainWindow(const QString &username)
{
    mainWindow = new MainWindow(username, networkManager);
    mainWindow->show();
    this->hide();
}

void TestClient::setupConnections()
{
    connect(networkManager, &ClientNetworkManager::messageReceived,
            this, &TestClient::onMessageReceived);
    connect(networkManager, &ClientNetworkManager::connected,
            this, &TestClient::onConnected);
    connect(networkManager, &ClientNetworkManager::disconnected,
            this, &TestClient::onDisconnected);
    connect(networkManager, &ClientNetworkManager::connectionError,
            this, [this](const QString &error) {
                ui->textEditOutput->append("错误: " + error);
                QMessageBox::warning(this, "连接错误", error);
            });
}

void TestClient::autoConnect()
{
    ui->textEditOutput->append("正在自动连接服务器...");
    networkManager->connectToServer("127.0.0.1", 8888);
}

void TestClient::on_loginButton_clicked()
{
    QString username = ui->usernameEdit->text().trimmed();
    QString password = ui->passwordEdit->text().trimmed();

    if (username.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入用户名");
        ui->usernameEdit->setFocus();
        return;
    }

    if (password.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入密码");
        ui->passwordEdit->setFocus();
        return;
    }

    if (!networkManager->isConnected()) {
        QMessageBox::warning(this, "连接错误", "请先连接服务器");
        return;
    }

    NetworkMessage msg;
    msg.type = LOGIN_REQUEST;
    msg.data["username"] = username;
    msg.data["password"] = password;
    networkManager->sendMessage(msg);

    ui->textEditOutput->append(QString("发送登录请求: %1").arg(username));
}

void TestClient::on_registerLinkButton_clicked()
{
    this->hide();
    RegisterDialog dialog(networkManager, this);
    connect(&dialog, &RegisterDialog::registrationSuccess, this, [this]() {
        QMessageBox::information(this, "提示", "注册成功，请登录");
    });
    dialog.exec();
    this->show();
    this->raise();
    this->activateWindow();
}

void TestClient::onMessageReceived(const NetworkMessage &message)
{
    QString displayText = QString("收到消息[类型:%1]: %2")
                              .arg(message.type)
                              .arg(QString::fromUtf8(QJsonDocument(message.data).toJson(QJsonDocument::Indented)));
    ui->textEditOutput->append(displayText);

    if (message.type == LOGIN_RESPONSE) {
        bool success = message.data["success"].toBool();
        QString resultMsg = message.data["message"].toString();

        if (success) {
            QString username = message.data["username"].toString();
            QMessageBox::information(this, "登录成功",
                                     QString("欢迎 %1！").arg(username));
            showMainWindow(username);
        } else {
            QMessageBox::warning(this, "登录失败", resultMsg);
        }
    }
}

void TestClient::onConnected()
{
    ui->textEditOutput->append("=== 连接服务器成功 ===");
}

void TestClient::onDisconnected()
{
    ui->textEditOutput->append("=== 与服务器断开连接 ===");
}

void TestClient::closeEvent(QCloseEvent *event)
{
    if (networkManager->isConnected() && !mainWindow) {
        ui->textEditOutput->append("程序关闭，断开服务器连接...");
        networkManager->disconnectFromServer();
    }
    QWidget::closeEvent(event);
}
