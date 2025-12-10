#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    static DatabaseManager& getInstance();
    bool initializeDatabase();

    // 用户相关操作
    bool validateUser(const QString &username, const QString &password);
    bool createUser(const QString &username, const QString &password, const QString &email = "");
    bool userExists(const QString &username);
    int getUserId(const QString &username);
    bool updateLastLogin(const QString &username);

    // 钱包相关
    bool queryUserBalance(const QString &username, double &balance);
    bool rechargeUserWallet(const QString &username, double amount, double &newBalance);
    bool deductFromWallet(const QString &username, double amount, double &newBalance);

    // 预订相关
    bool processBooking(const QString &username, int flightId, int cabinId,
                        const QString &passengerName, const QString &passengerId,
                        const QString &passengerPhone, double totalPrice,
                        QString &bookingNumber, double &newBalance);

    // 航班和舱位查询
    bool getFlightAvailableSeats(int flightId, int &availableSeats);
    bool getCabinAvailableSeats(int cabinId, int &availableSeats);

private:
    DatabaseManager(QObject *parent = nullptr);
    bool openDatabase();
};

#endif // DATABASEMANAGER_H
