#include "testclient.h"
#include "ui_testclient.h"
#include "clientnetworkmanager.h"
#include "../Common/protocol.h"
#include <QJsonDocument>
#include <QDebug>
#include <QMessageBox>

TestClient::TestClient(QWidget *parent)
    : QWidget(parent),
    ui(new Ui::TestClient),
    networkManager(new ClientNetworkManager(this))
{
    ui->setupUi(this);
    setupConnections();
}

TestClient::~TestClient()
{
    delete ui;
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

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入用户名和密码");
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

void TestClient::on_registerButton_clicked()
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
}

void TestClient::onMessageReceived(const NetworkMessage &message)
{
    QString displayText = QString("收到消息[类型:%1]: %2")
                              .arg(message.type)
                              .arg(QString::fromUtf8(QJsonDocument(message.data).toJson(QJsonDocument::Indented)));
    ui->textEditOutput->append(displayText);

    // 处理登录响应
    if (message.type == LOGIN_RESPONSE) {
        bool success = message.data["success"].toBool();
        QString resultMsg = message.data["message"].toString();

        if (success) {
            QString username = message.data["username"].toString();
            QMessageBox::information(this, "登录成功",
                                     QString("欢迎 %1！").arg(username));
        } else {
            QMessageBox::warning(this, "登录失败", resultMsg);
        }
    }

    // 处理注册响应
    if (message.type == REGISTER_RESPONSE) {
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
