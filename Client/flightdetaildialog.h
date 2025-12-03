#ifndef FLIGHTDETAILDIALOG_H
#define FLIGHTDETAILDIALOG_H

#include <QDialog>
#include <QJsonArray>  // 添加这个头文件
#include "../Common/flight.h"
#include "../Common/cabin.h"
#include "clientnetworkmanager.h"

namespace Ui {
class FlightDetailDialog;
}

class FlightDetailDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FlightDetailDialog(const Flight &flight,
                                ClientNetworkManager *networkManager,
                                const QString &username,
                                QWidget *parent = nullptr);
    ~FlightDetailDialog();

private slots:
    void onBookButtonClicked(int row);
    void onCloseButtonClicked();
    void onMessageReceived(const NetworkMessage &message);

private:
    void setupUI();
    void loadCabinData();
    void displayCabinData(const QList<Cabin> &cabins);
    void showBookingDialog(const Cabin &cabin);

    Ui::FlightDetailDialog *ui;
    Flight flight;
    ClientNetworkManager *networkManager;
    QString currentUsername;
    QList<Cabin> currentCabins;
};

#endif // FLIGHTDETAILDIALOG_H
