#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QTimer>
#include <QJsonArray>
#include <QDateTime>
#include <QDate>
#include <QButtonGroup>
#include <QPushButton>
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
    void logoutRequested();

private slots:
    void onSearchButtonClicked();
    void onFlightItemDoubleClicked(QListWidgetItem *item);
    void onMessageReceived(const NetworkMessage &message);
    void onLogoutButtonClicked();
    void onSwapButtonClicked();
    void onAirlineFilterChanged(int index);
    void onDateButtonClicked();
    void onPrevWeekClicked();
    void onNextWeekClicked();
    void onCalendarButtonClicked();

private:
    void setupConnections();
    void searchFlights();
    void displayFlights(const QList<Flight> &flights);
    void addFlightItem(const Flight &flight);
    void showFlightDetail(const Flight &flight);
    void setupDateSelection();
    void updateDateButtons();
    void searchFlightsByDate(const QDate &date);
    void showCalendarDialog();
    QString getSearchConditions() const;

    Ui::MainWindow *ui;
    QString currentUsername;
    ClientNetworkManager* networkManager;
    QList<Flight> currentFlights;
    QDate currentCenterDate;
    QDate selectedDate;
    QButtonGroup *dateButtonGroup;
    QList<QPushButton*> dateButtons;
    QDate currentStartDate;
};

#endif // MAINWINDOW_H
