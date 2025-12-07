#ifndef FLIGHTDETAILDIALOG_H
#define FLIGHTDETAILDIALOG_H

#include <QDialog>
#include <QJsonArray>
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
    void displayFilteredCabins();  // 显示筛选后的舱位数据
    void createCabinSelectionButtons();  // 创建仓位选择按钮
    void showBookingDialog(const Cabin &cabin);

    Ui::FlightDetailDialog *ui;
    Flight flight;
    ClientNetworkManager *networkManager;
    QString currentUsername;
    QList<Cabin> currentCabins;

    QMap<int, QString> cabinTypes;  // 仓位类型ID -> 名称
    QMap<int, int> cabinPrices;     // 仓位类型ID -> 价格
    QMap<int, int> cabinAvailable;  // 仓位类型ID -> 剩余座位数
    int selectedCabinType;          // 当前选择的仓位类型
};

#endif // FLIGHTDETAILDIALOG_H
