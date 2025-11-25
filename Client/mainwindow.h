#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QTimer>           // 添加QTimer头文件
#include <QJsonArray>       // 添加QJsonArray头文件
#include <QDateTime>        // 添加QDateTime头文件
#include "clientnetworkmanager.h"
#include "../Common/flight.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(const QString &username, ClientNetworkManager* networkManager = nullptr, QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void logoutRequested();  // 添加登出信号

private slots:
    void onSearchButtonClicked();
    void onSortChanged(int index);
    void onFlightItemDoubleClicked(QListWidgetItem *item);
    void onMessageReceived(const NetworkMessage &message);
    void onLogoutButtonClicked();
    void onSwapButtonClicked();

private:
    void setupConnections();
    void searchFlights();
    void displayFlights(const QList<Flight> &flights);
    void addFlightItem(const Flight &flight);
    void showFlightDetail(const Flight &flight);

    Ui::MainWindow *ui;
    QString currentUsername;
    ClientNetworkManager* networkManager;
    QList<Flight> currentFlights;
};

#endif // MAINWINDOW_H
