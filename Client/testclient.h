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

class TestClient : public QWidget
{
    Q_OBJECT

public:
    explicit TestClient(QWidget *parent = nullptr);
    ~TestClient();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void on_loginButton_clicked();
    void on_registerLinkButton_clicked();

    void onMessageReceived(const NetworkMessage &message);
    void onConnected();
    void onDisconnected();
    void showLoginWindow();

private:
    void setupConnections();
    void autoConnect();
    void showMainWindow(const QString &username);

    // 新增方法声明
    void setBackgroundImage();
    void setupIcons();

    Ui::TestClient *ui;
    ClientNetworkManager *networkManager;
    RegisterDialog *registerDialog;
    MainWindow *mainWindow;
};

#endif // TESTCLIENT_H
