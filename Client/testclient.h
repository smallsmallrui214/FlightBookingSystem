#ifndef TESTCLIENT_H
#define TESTCLIENT_H

#include <QWidget>
#include "registerdialog.h"
#include "mainwindow.h"

// 前向声明
namespace Ui {
class TestClient;
}

class ClientNetworkManager;
class NetworkMessage;
//class RegisterDialog;
//class MainWindow;

class TestClient : public QWidget
{
    Q_OBJECT

public:
    explicit TestClient(QWidget *parent = nullptr);
    ~TestClient();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void on_connectButton_clicked();
    void on_disconnectButton_clicked();
    void on_loginButton_clicked();      // 登录按钮
    void on_registerButton_clicked();   // 注册按钮
    void on_registerLinkButton_clicked();// 新增注册链接按钮槽函数

    void onMessageReceived(const NetworkMessage &message);
    void onConnected();
    void onDisconnected();
    void showLoginWindow();  // 新增：显示登录界面

private:
    void setupConnections();
    void autoConnect();
    void showMainWindow(const QString &username);


    Ui::TestClient *ui;
    ClientNetworkManager *networkManager;
    RegisterDialog *registerDialog;  // 新增注册对话框指针
    MainWindow *mainWindow;  // 新增：主界面指针
};

#endif // TESTCLIENT_H
