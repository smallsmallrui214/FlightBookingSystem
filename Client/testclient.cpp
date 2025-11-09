#include "testclient.h"
#include "ui_testclient.h"
#include "clientnetworkmanager.h"
#include "../Common/protocol.h"
#include <QJsonDocument>
#include <QDebug>

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

void TestClient::on_sendButton_clicked()
{
    QString content = ui->textEditInput->toPlainText().trimmed();
    if (content.isEmpty()) {
        content = "Hello Server!";
    }

    NetworkMessage msg;
    msg.type = TEST_MESSAGE;
    msg.data["content"] = content;
    networkManager->sendMessage(msg);

    ui->textEditOutput->append("发送测试消息: " + content);
    ui->textEditInput->clear();
}

void TestClient::on_loginButton_clicked()
{
    NetworkMessage msg;
    msg.type = LOGIN_REQUEST;
    msg.data["username"] = "admin";
    msg.data["password"] = "123456";
    networkManager->sendMessage(msg);

    ui->textEditOutput->append("发送登录请求: admin / 123456");
}

void TestClient::onMessageReceived(const NetworkMessage &message)
{
    QString displayText = QString("收到消息[类型:%1]: %2")
                              .arg(message.type)
                              .arg(QString::fromUtf8(QJsonDocument(message.data).toJson(QJsonDocument::Indented)));
    ui->textEditOutput->append(displayText);
}

void TestClient::onConnected()
{
    ui->textEditOutput->append("=== 连接服务器成功 ===");
}

void TestClient::onDisconnected()
{
    ui->textEditOutput->append("=== 与服务器断开连接 ===");
}
