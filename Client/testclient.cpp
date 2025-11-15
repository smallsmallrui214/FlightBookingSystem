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

class PlaceholderFilter : public QObject
{
public:
    PlaceholderFilter(QLineEdit *edit, const QString &text)
        : QObject(edit), lineEdit(edit), placeholder(text) {}

protected:
    bool eventFilter(QObject *obj, QEvent *event) override {
        if (obj == lineEdit) {
            if (event->type() == QEvent::FocusIn) {
                lineEdit->setPlaceholderText(""); // 获得焦点隐藏占位符
            } else if (event->type() == QEvent::FocusOut) {
                if (lineEdit->text().isEmpty())
                    lineEdit->setPlaceholderText(placeholder); // 失去焦点且为空显示占位符
            }
        }
        return false; // 继续处理事件
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


    ui->outputLabel->hide();
    ui->textEditOutput->hide();

    //ui->registerGroup->hide();


    // 清除可能的初始文本
    ui->usernameEdit->clear();
    ui->passwordEdit->clear();
    //ui->regUsernameEdit->clear();
    //ui->regPasswordEdit->clear();
    //ui->emailEdit->clear();


    // 设置占位符文本（点击时会自动隐藏）
    // 设置占位符文本，并安装焦点事件过滤器
    ui->usernameEdit->setPlaceholderText("请输入用户名");
    ui->usernameEdit->installEventFilter(new PlaceholderFilter(ui->usernameEdit, "请输入用户名"));

    ui->passwordEdit->setPlaceholderText("请输入密码");
    ui->passwordEdit->installEventFilter(new PlaceholderFilter(ui->passwordEdit, "请输入密码"));

    //ui->regUsernameEdit->setPlaceholderText("请输入用户名");
    //ui->regUsernameEdit->installEventFilter(new PlaceholderFilter(ui->regUsernameEdit, "请输入用户名"));

    //ui->regPasswordEdit->setPlaceholderText("请输入密码");
    //ui->regPasswordEdit->installEventFilter(new PlaceholderFilter(ui->regPasswordEdit, "请输入密码"));

    //ui->emailEdit->setPlaceholderText("请输入邮箱(可选)");
    //ui->emailEdit->installEventFilter(new PlaceholderFilter(ui->emailEdit, "请输入邮箱(可选)"));






    // 创建注册链接按钮
    QPushButton *registerLinkButton = new QPushButton("没有账号？立即注册", this);
    registerLinkButton->setObjectName("registerLinkButton");
    registerLinkButton->setStyleSheet("color: blue; text-decoration: underline; border: none; background: transparent;");//颜色，下划线，透明背景



    // 将注册链接按钮添加到登录区域的布局中
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(registerLinkButton);
    buttonLayout->addStretch();  // 添加弹性空间
    buttonLayout->addWidget(ui->loginButton);  // 原来的登录按钮


    // 找到登录区域的布局并添加按钮行
    QVBoxLayout *loginLayout = qobject_cast<QVBoxLayout*>(ui->loginGroup->layout());
    if (loginLayout) {
        loginLayout->addLayout(buttonLayout);
    }


    // 连接注册链接按钮的信号
    connect(registerLinkButton, &QPushButton::clicked, this, &TestClient::on_registerLinkButton_clicked);


    setupConnections();

    QTimer::singleShot(100, this, [this]() {
        autoConnect();
    });
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


// 显示主界面的函数
void TestClient::showMainWindow(const QString &username)
{
    // 创建主界面
    mainWindow = new MainWindow(username);
    mainWindow->show();

    // 关闭登录界面
    this->close();
}


void TestClient::setupConnections()
{
    // 连接网络管理器的信号
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

void TestClient::on_connectButton_clicked()
{
    ui->textEditOutput->append("正在连接服务器...");
    networkManager->connectToServer("127.0.0.1", 8888);
}

void TestClient::on_disconnectButton_clicked()
{
    ui->textEditOutput->append("断开服务器连接...");
    networkManager->disconnectFromServer();
}

void TestClient::on_loginButton_clicked()
{
    QString username = ui->usernameEdit->text().trimmed();
    QString password = ui->passwordEdit->text().trimmed();

    // 检查用户名是否为空
    if (username.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入用户名");
        ui->usernameEdit->setFocus(); // 聚焦到用户名输入框
        return;
    }

    // 检查密码是否为空
    if (password.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入密码");
        ui->passwordEdit->setFocus(); // 聚焦到密码输入框
        return;
    }

    // 首先检查是否是内置测试账号
    if (username == "test" && password == "123456") {
        // 内置测试账号，直接登录成功，不发送到服务器
        QMessageBox::information(this, "登录成功", "欢迎测试用户！");
        showMainWindow(username);
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
    // 隐藏登录界面
    this->hide();

    // 创建注册对话框（模态对话框）
    RegisterDialog dialog(networkManager, this);

    // 注册成功
    connect(&dialog, &RegisterDialog::registrationSuccess, this, [this]() {
        QMessageBox::information(this, "提示", "注册成功，请登录");
    });

    // 显示模态对话框（会自动阻塞，关闭后继续执行）
    dialog.exec();

    // 对话框关闭后，显示登录界面
    this->show();
    this->raise();
    this->activateWindow();
}

/*void TestClient::on_registerButton_clicked()
{
    QString username = ui->regUsernameEdit->text().trimmed();
    QString password = ui->regPasswordEdit->text().trimmed();
    QString email = ui->emailEdit->text().trimmed();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入用户名和密码");
        return;
    }

    if (!networkManager->isConnected()) {
        QMessageBox::warning(this, "连接错误", "请先连接服务器");
        return;
    }

    NetworkMessage msg;
    msg.type = REGISTER_REQUEST;
    msg.data["username"] = username;
    msg.data["password"] = password;
    msg.data["email"] = email;
    networkManager->sendMessage(msg);

    ui->textEditOutput->append(QString("发送注册请求: %1").arg(username));
}*/

void TestClient::onMessageReceived(const NetworkMessage &message)
{
    QString displayText = QString("收到消息[类型:%1]: %2")
                              .arg(message.type)
                              .arg(QString::fromUtf8(QJsonDocument(message.data).toJson(QJsonDocument::Indented)));
    ui->textEditOutput->append(displayText);

    // 处理登录响应
    if (message.type == LOGIN_RESPONSE) {

        // 正常处理服务器返回的登录响应
        bool success = message.data["success"].toBool();
        QString resultMsg = message.data["message"].toString();

        if (success) {
            QString username = message.data["username"].toString();
            QMessageBox::information(this, "登录成功",
                                     QString("欢迎 %1！").arg(username));
            // 登录成功，跳转到主界面
            showMainWindow(username);
        } else {
            QMessageBox::warning(this, "登录失败", resultMsg);
        }
    }

    // 处理注册响应
    /*if (message.type == REGISTER_RESPONSE) {
        bool success = message.data["success"].toBool();
        QString resultMsg = message.data["message"].toString();

        if (success) {
            QString username = message.data["username"].toString();
            QMessageBox::information(this, "注册成功",
                                     QString("用户 %1 注册成功").arg(username));
            // 清空注册表单
            ui->regUsernameEdit->clear();
            ui->regPasswordEdit->clear();
            ui->emailEdit->clear();
        } else {
            QMessageBox::warning(this, "注册失败", resultMsg);
        }
    }*/
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
    // 在窗口关闭时断开连接
    if (networkManager->isConnected()) {
        ui->textEditOutput->append("程序关闭，断开服务器连接...");
        networkManager->disconnectFromServer();
    }

    QWidget::closeEvent(event);
}
