#include <QCoreApplication>
#include <QDebug>
#include "servernetworkmanager.h"
#include "databasemanager.h"

class TestServer : public QObject
{
    Q_OBJECT

public:
    TestServer() {
        // 初始化数据库连接
        if (!DatabaseManager::getInstance().initializeDatabase()) {
            qDebug() << "数据库连接失败!";
            return;
        }

        connect(&networkManager, &ServerNetworkManager::messageReceived,
                this, &TestServer::onMessageReceived);
        connect(&networkManager, &ServerNetworkManager::clientConnected,
                this, &TestServer::onClientConnected);
    }

    void start() {
        if (networkManager.startServer(8888)) {
            qDebug() << "=== 航班服务器已启动 ===";
            qDebug() << "端口: 8888";
            qDebug() << "等待客户端连接...";
        }
    }

private slots:
    void onClientConnected(QTcpSocket *client) {
        qDebug() << "客户端连接:" << client->peerAddress().toString();

        NetworkMessage welcomeMsg;
        welcomeMsg.type = TEST_MESSAGE;
        welcomeMsg.data["content"] = "欢迎连接到航班服务器!";
        networkManager.sendMessage(welcomeMsg, client);
    }

    void onMessageReceived(const NetworkMessage &message, QTcpSocket *client) {
        qDebug() << "收到消息类型:" << message.type;

        switch (message.type) {
        case TEST_MESSAGE: {
            QString content = message.data["content"].toString();
            qDebug() << "测试消息:" << content;

            NetworkMessage reply;
            reply.type = TEST_MESSAGE;
            reply.data["content"] = "服务器回复: " + content;
            networkManager.sendMessage(reply, client);
            break;
        }
        case LOGIN_REQUEST: {
            QString username = message.data["username"].toString();
            QString password = message.data["password"].toString();

            DatabaseManager& db = DatabaseManager::getInstance();
            NetworkMessage reply;
            reply.type = LOGIN_RESPONSE;

            if (db.validateUser(username, password)) {
                int userId = db.getUserId(username);
                reply.data["success"] = true;
                reply.data["message"] = "登录成功";
                reply.data["user_id"] = userId;
                reply.data["username"] = username;
                qDebug() << "用户登录成功:" << username << "ID:" << userId;
            } else {
                reply.data["success"] = false;
                reply.data["message"] = "用户名或密码错误";
                qDebug() << "用户登录失败:" << username;
            }
            networkManager.sendMessage(reply, client);
            break;
        }
        case REGISTER_REQUEST: {
            QString username = message.data["username"].toString();
            QString password = message.data["password"].toString();
            QString email = message.data["email"].toString();

            DatabaseManager& db = DatabaseManager::getInstance();
            NetworkMessage reply;
            reply.type = REGISTER_RESPONSE;

            if (db.createUser(username, password, email)) {
                reply.data["success"] = true;
                reply.data["message"] = "注册成功";
                reply.data["username"] = username;
                qDebug() << "用户注册成功:" << username;
            } else {
                reply.data["success"] = false;
                reply.data["message"] = "注册失败，用户名可能已存在";
                qDebug() << "用户注册失败:" << username;
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

#include "main.moc"
