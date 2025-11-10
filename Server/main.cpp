#include <QCoreApplication>
#include <QDebug>
#include "servernetworkmanager.h"

class TestServer : public QObject
{
    Q_OBJECT

public:
    TestServer() {
        connect(&networkManager, &ServerNetworkManager::messageReceived,
                this, &TestServer::onMessageReceived);
        connect(&networkManager, &ServerNetworkManager::clientConnected,
                this, &TestServer::onClientConnected);
    }

    void start() {
        if (networkManager.startServer(8888)) {
            qDebug() << "=== 测试服务器已启动 ===";
            qDebug() << "等待客户端连接...";
        }
    }

private slots:
    void onClientConnected(QTcpSocket *client) {
        qDebug() << "有新客户端连接，发送欢迎消息";

        // 发送欢迎消息
        NetworkMessage welcomeMsg;
        welcomeMsg.type = TEST_MESSAGE;
        welcomeMsg.data["content"] = "欢迎连接到航班服务器!";
        networkManager.sendMessage(welcomeMsg, client);
    }

    void onMessageReceived(const NetworkMessage &message, QTcpSocket *client) {
        qDebug() << "处理客户端消息，类型:" << message.type;

        switch (message.type) {
        case TEST_MESSAGE: {
            QString content = message.data["content"].toString();
            qDebug() << "收到测试消息:" << content;

            // 回复消息
            NetworkMessage reply;
            reply.type = TEST_MESSAGE;
            reply.data["content"] = "服务器已收到你的消息: " + content;
            networkManager.sendMessage(reply, client);
            break;
        }
        case LOGIN_REQUEST: {
            QString username = message.data["username"].toString();
            QString password = message.data["password"].toString();
            qDebug() << "登录请求 - 用户名:" << username << "密码:" << password;

            // 模拟登录验证
            NetworkMessage reply;
            reply.type = LOGIN_RESPONSE;
            if (username == "admin" && password == "123456") {
                reply.data["success"] = true;
                reply.data["message"] = "登录成功";
                reply.data["user_id"] = 1;
            } else {
                reply.data["success"] = false;
                reply.data["message"] = "用户名或密码错误";
            }
            networkManager.sendMessage(reply, client);
            break;
        }
        default:
            qDebug() << "未知消息类型:" << message.type;
        }
    }

private:
    ServerNetworkManager networkManager;
};

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    TestServer server;
    server.start();

    return a.exec();
}

#include "main.moc"  // 添加这一行
//lec到此一游
