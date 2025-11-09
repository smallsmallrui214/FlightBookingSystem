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
    void on_sendButton_clicked();
    void on_loginButton_clicked();

    void onMessageReceived(const NetworkMessage &message);
    void onConnected();
    void onDisconnected();

private:
    void setupConnections();

    Ui::TestClient *ui;
    ClientNetworkManager *networkManager;
};

#endif // TESTCLIENT_H
