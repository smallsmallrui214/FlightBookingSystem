#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QMutex>
#include <QDateTime>
#include <QThread>
#include <QCoreApplication>
#include <functional>

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

    // 新增方法
    QJsonArray getRechargeRecords(const QString &username);

private:
    DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    QSqlDatabase getNewConnection();
    void safeCloseConnection(QSqlDatabase &db);
    bool executeWithTransaction(QSqlDatabase &db, std::function<bool(QSqlDatabase&)> operation);

    // 删除复制构造函数和赋值运算符
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    QMutex connectionMutex;
    static int connectionCounter;
};

#endif // DATABASEMANAGER_H
