#ifndef TESTCLIENT_H
#define TESTCLIENT_H

#include <QWidget>

// 前向声明
namespace Ui {
class TestClient;
}

class ClientNetworkManager;
class NetworkMessage;

class TestClient : public QWidget
{
    Q_OBJECT

public:
    explicit TestClient(QWidget *parent = nullptr);
    ~TestClient();

private slots:
    void on_connectButton_clicked();
    void on_disconnectButton_clicked();
    void on_loginButton_clicked();      // 登录按钮
    void on_registerButton_clicked();   // 注册按钮

    void onMessageReceived(const NetworkMessage &message);
    void onConnected();
    void onDisconnected();

private:
    void setupConnections();

    Ui::TestClient *ui;
    ClientNetworkManager *networkManager;
};

#endif // TESTCLIENT_H
