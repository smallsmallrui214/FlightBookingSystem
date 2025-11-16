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
    this->setFixedSize(640, 500);

    QString messageBoxStyle =
        "QMessageBox {"
        "    background-color: #2c3e50;"  // 背景色
        "    color: #cccccc;"             // 文字颜色改为浅灰色
        "    font-size: 14px;"
        "}"
        "QMessageBox QLabel {"
        "    color: #cccccc;"             // 消息文本颜色
        "    background-color: transparent;"
        "    font-size: 14px;"
        "}"
        "QMessageBox QPushButton {"
        "    background-color: #3498db;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 5px;"
        "    padding: 8px 15px;"
        "    font-size: 12px;"
        "    min-width: 80px;"
        "}"
        "QMessageBox QPushButton:hover {"
        "    background-color: #5dade2;"
        "}"
        "QMessageBox QPushButton:pressed {"
        "    background-color: #2e86c1;"
        "}";

    // 应用样式到所有QMessageBox
    setStyleSheet(messageBoxStyle);


    ui->outputLabel->hide();
    ui->textEditOutput->hide();


    // 清除可能的初始文本
    ui->usernameEdit->clear();
    ui->passwordEdit->clear();
    //ui->regUsernameEdit->clear();
    //ui->regPasswordEdit->clear();
    //ui->emailEdit->clear();
    ui->loginButton->setFixedSize(80, 15);
    ui->loginButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    // 设置占位符文本（点击时会自动隐藏）
    // 设置占位符文本，并安装焦点事件过滤器
    ui->usernameEdit->setPlaceholderText("请输入用户名");
    ui->usernameEdit->installEventFilter(new PlaceholderFilter(ui->usernameEdit, "请输入用户名"));

    ui->passwordEdit->setPlaceholderText("请输入密码");
    ui->passwordEdit->installEventFilter(new PlaceholderFilter(ui->passwordEdit, "请输入密码"));


    // 创建注册链接按钮
    QPushButton *registerLinkButton = new QPushButton("没有账号？立即注册", this);
    registerLinkButton->setObjectName("registerLinkButton");
    // 设置链接按钮样式
    registerLinkButton->setStyleSheet(
        "QPushButton {"
        "  background-color: transparent;"
        "  color: #3498db;"
        "  border: none;"
        "  text-decoration: underline;"
        "  padding: 5px;"
        "  font-size: 12px;"
        "  min-height: 25px;"
        "}"
        "QPushButton:hover {"
        "  color: #5dade2;"
        "  background-color: rgba(52, 152, 219, 0.1);"
        "}"
        );



    // ✅ 将注册链接按钮添加到布局中
    // 找到buttonLayout
    QHBoxLayout *buttonLayout = qobject_cast<QHBoxLayout*>(ui->buttonLayout);
    if (buttonLayout) {
        // 在spacer之前插入注册链接按钮
        buttonLayout->insertWidget(0, registerLinkButton);

        // 可选：调整spacer的大小，为按钮留出空间
        QSpacerItem *spacer = buttonLayout->itemAt(1)->spacerItem();
        if (spacer) {
            spacer->changeSize(20, 20); // 减小spacer大小
        }
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
    this->hide();

    // 创建注册对话框
    RegisterDialog dialog(networkManager, this);

    connect(&dialog, &RegisterDialog::registrationSuccess, this, [this]() {
        QMessageBox::information(this, "提示", "注册成功，请登录");
    });


    dialog.exec();  // 对话框关闭后，下面的代码立即执行


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
