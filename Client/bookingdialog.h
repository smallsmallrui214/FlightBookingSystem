#ifndef BOOKINGDIALOG_H
#define BOOKINGDIALOG_H

#include <QDialog>
#include "../Common/flight.h"
#include "../Common/cabin.h"
#include "clientnetworkmanager.h"

namespace Ui {
class BookingDialog;
}

class BookingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BookingDialog(const Flight &flight, const Cabin &cabin,
                           const QString &username,
                           ClientNetworkManager *networkManager,
                           QWidget *parent = nullptr);
    ~BookingDialog();

private slots:
    void onBookButtonClicked();
    void onCancelButtonClicked();
    void onMessageReceived(const NetworkMessage &message);

private:
    bool validateInput();
    void showSuccessMessage(const QString &bookingNumber);

    Ui::BookingDialog *ui;
    Flight flight;
    Cabin cabin;
    QString username;
    ClientNetworkManager *networkManager;
};

#endif // BOOKINGDIALOG_H
