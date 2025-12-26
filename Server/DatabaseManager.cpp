#include "databasemanager.h"

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
{
}

DatabaseManager& DatabaseManager::getInstance()
{
    static DatabaseManager instance;
    return instance;
}

bool DatabaseManager::openDatabase()
{
    // 最简单的方式：使用ODBC连接
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");
    db.setDatabaseName("flight_booking_system"); // 数据库名
    db.setUserName("root");                      // 用户名
    db.setPassword("LR20060214");                    // 密码

    if (!db.open()) {
        qDebug() << "数据库连接失败:" << db.lastError().text();
        return false;
    }

    qDebug() << "数据库连接成功!";
    return true;
}

bool DatabaseManager::initializeDatabase()
{
    // 直接连接，不进行测试查询
    if (!openDatabase()) {
        return false;
    }

    qDebug() << "数据库初始化完成!";
    return true;
}

bool DatabaseManager::validateUser(const QString &username, const QString &password)
{
    QSqlQuery query;
    query.prepare("SELECT id FROM users WHERE username = ? AND password = ?");
    query.addBindValue(username);
    query.addBindValue(password);

    if (!query.exec()) {
        qDebug() << "用户验证失败:" << query.lastError().text();
        return false;
    }

    bool isValid = query.next();
    qDebug() << "用户验证:" << username << (isValid ? "成功" : "失败");
    return isValid;
}

bool DatabaseManager::userExists(const QString &username)
{
    QSqlQuery query;
    query.prepare("SELECT id FROM users WHERE username = ?");
    query.addBindValue(username);

    if (!query.exec()) {
        qDebug() << "检查用户存在失败:" << query.lastError().text();
        return false;
    }

    return query.next();
}

bool DatabaseManager::createUser(const QString &username, const QString &password, const QString &email)
{
    // 检查用户名是否已存在
    if (userExists(username)) {
        qDebug() << "创建用户失败: 用户名" << username << "已存在";
        return false;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO users (username, password, email) VALUES (?, ?, ?)");
    query.addBindValue(username);
    query.addBindValue(password);
    query.addBindValue(email);

    if (!query.exec()) {
        qDebug() << "创建用户失败:" << query.lastError().text();
        return false;
    }

    qDebug() << "用户创建成功:" << username;
    return true;
}

int DatabaseManager::getUserId(const QString &username)
{
    QSqlQuery query;
    query.prepare("SELECT id FROM users WHERE username = ?");
    query.addBindValue(username);

    if (!query.exec() || !query.next()) {
        return -1;
    }

    return query.value(0).toInt();
}

bool DatabaseManager::updateLastLogin(const QString &username)
{
    QSqlQuery query;
    query.prepare("UPDATE users SET last_login = NOW() WHERE username = ?");
    query.addBindValue(username);

    if (!query.exec()) {
        qDebug() << "更新最后登录时间失败:" << query.lastError().text();
        qDebug() << "SQL错误:" << query.lastError().databaseText();
        return false;
    }

    if (query.numRowsAffected() > 0) {
        qDebug() << "成功更新用户最后登录时间:" << username;
        return true;
    } else {
        qDebug() << "未找到用户或更新失败:" << username;
        return false;
    }
}

// databasemanager.cpp - 添加以下方法实现

bool DatabaseManager::queryUserBalance(const QString &username, double &balance)
{
    QSqlQuery query;
    query.prepare("SELECT w.balance FROM wallets w "
                  "JOIN users u ON w.user_id = u.id "
                  "WHERE u.username = :username");
    query.bindValue(":username", username);

    if (query.exec() && query.next()) {
        balance = query.value(0).toDouble();
        qDebug() << "查询用户" << username << "余额成功:" << balance;
        return true;
    }

    qDebug() << "查询用户余额失败:" << query.lastError().text()
             << "用户:" << username;
    return false;
}

bool DatabaseManager::rechargeUserWallet(const QString &username, double amount, double &newBalance)
{
    int userId = getUserId(username);
    if (userId <= 0) {
        qDebug() << "用户不存在:" << username;
        return false;
    }

    QSqlDatabase::database().transaction();

    try {
        // 1. 查询当前余额
        QSqlQuery query;
        query.prepare("SELECT balance FROM wallets WHERE user_id = :user_id FOR UPDATE");
        query.bindValue(":user_id", userId);

        if (!query.exec() || !query.next()) {
            qDebug() << "查询钱包失败:" << query.lastError().text();
            QSqlDatabase::database().rollback();
            return false;
        }

        double oldBalance = query.value(0).toDouble();
        newBalance = oldBalance + amount;

        // 2. 更新余额
        query.prepare("UPDATE wallets SET balance = :new_balance WHERE user_id = :user_id");
        query.bindValue(":new_balance", newBalance);
        query.bindValue(":user_id", userId);

        if (!query.exec()) {
            qDebug() << "更新余额失败:" << query.lastError().text();
            QSqlDatabase::database().rollback();
            return false;
        }

        // 3. 记录充值历史
        query.prepare("INSERT INTO recharge_records (user_id, amount, old_balance, new_balance) "
                      "VALUES (:user_id, :amount, :old_balance, :new_balance)");
        query.bindValue(":user_id", userId);
        query.bindValue(":amount", amount);
        query.bindValue(":old_balance", oldBalance);
        query.bindValue(":new_balance", newBalance);

        if (!query.exec()) {
            qDebug() << "记录充值历史失败:" << query.lastError().text();
            QSqlDatabase::database().rollback();
            return false;
        }

        QSqlDatabase::database().commit();
        qDebug() << "用户" << username << "充值成功，金额:" << amount
                 << "，新余额:" << newBalance;
        return true;

    } catch (...) {
        QSqlDatabase::database().rollback();
        qDebug() << "充值过程中发生异常";
        return false;
    }
}

bool DatabaseManager::processBooking(const QString &username, int flightId, int cabinId,
                                     const QString &passengerName, const QString &passengerId,
                                     const QString &passengerPhone, double totalPrice,
                                     QString &bookingNumber, double &newBalance)
{
    int userId = getUserId(username);
    if (userId <= 0) {
        qDebug() << "用户不存在:" << username;
        return false;
    }

    QSqlDatabase::database().transaction();

    try {
        // 1. 检查余额是否足够
        double currentBalance = 0;
        if (!queryUserBalance(username, currentBalance)) {
            QSqlDatabase::database().rollback();
            return false;
        }

        if (currentBalance < totalPrice) {
            qDebug() << "余额不足，需要:" << totalPrice << "，当前:" << currentBalance;
            QSqlDatabase::database().rollback();
            return false;
        }

        // 2. 检查航班和舱位可用座位
        int flightSeats = 0, cabinSeats = 0;
        QSqlQuery query;

        query.prepare("SELECT available_seats FROM flights WHERE id = :flight_id");
        query.bindValue(":flight_id", flightId);
        if (!query.exec() || !query.next()) {
            qDebug() << "查询航班座位失败:" << query.lastError().text();
            QSqlDatabase::database().rollback();
            return false;
        }
        flightSeats = query.value(0).toInt();

        query.prepare("SELECT available_seats FROM cabins WHERE id = :cabin_id");
        query.bindValue(":cabin_id", cabinId);
        if (!query.exec() || !query.next()) {
            qDebug() << "查询舱位座位失败:" << query.lastError().text();
            QSqlDatabase::database().rollback();
            return false;
        }
        cabinSeats = query.value(0).toInt();

        if (flightSeats <= 0 || cabinSeats <= 0) {
            qDebug() << "座位已售罄，航班座位:" << flightSeats << "，舱位座位:" << cabinSeats;
            QSqlDatabase::database().rollback();
            return false;
        }

        // 3. 扣款
        newBalance = currentBalance - totalPrice;
        query.prepare("UPDATE wallets SET balance = :new_balance WHERE user_id = :user_id");
        query.bindValue(":new_balance", newBalance);
        query.bindValue(":user_id", userId);

        if (!query.exec()) {
            qDebug() << "扣款失败:" << query.lastError().text();
            QSqlDatabase::database().rollback();
            return false;
        }

        // 4. 更新航班座位
        query.prepare("UPDATE flights SET available_seats = available_seats - 1 WHERE id = :flight_id");
        query.bindValue(":flight_id", flightId);
        if (!query.exec()) {
            qDebug() << "更新航班座位失败:" << query.lastError().text();
            QSqlDatabase::database().rollback();
            return false;
        }

        // 5. 更新舱位座位
        query.prepare("UPDATE cabins SET available_seats = available_seats - 1 WHERE id = :cabin_id");
        query.bindValue(":cabin_id", cabinId);
        if (!query.exec()) {
            qDebug() << "更新舱位座位失败:" << query.lastError().text();
            QSqlDatabase::database().rollback();
            return false;
        }

        // 6. 创建预订记录
        query.prepare("INSERT INTO bookings (user_id, flight_id, cabin_id, passenger_name, "
                      "passenger_id, passenger_phone, total_price) "
                      "VALUES (:user_id, :flight_id, :cabin_id, :passenger_name, "
                      ":passenger_id, :passenger_phone, :total_price)");
        query.bindValue(":user_id", userId);
        query.bindValue(":flight_id", flightId);
        query.bindValue(":cabin_id", cabinId);
        query.bindValue(":passenger_name", passengerName);
        query.bindValue(":passenger_id", passengerId);
        query.bindValue(":passenger_phone", passengerPhone);
        query.bindValue(":total_price", totalPrice);

        if (!query.exec()) {
            qDebug() << "创建预订记录失败:" << query.lastError().text();
            QSqlDatabase::database().rollback();
            return false;
        }

        // 7. 获取预订号（最后插入的ID）
        bookingNumber = QString::number(query.lastInsertId().toInt());

        QSqlDatabase::database().commit();

        qDebug() << "预订成功！用户:" << username
                 << "，预订号:" << bookingNumber
                 << "，金额:" << totalPrice
                 << "，新余额:" << newBalance;

        return true;

    } catch (...) {
        QSqlDatabase::database().rollback();
        qDebug() << "预订过程中发生异常";
        return false;
    }
}
// 在 processBooking 方法后添加

bool DatabaseManager::deductFromWallet(const QString &username, double amount, double &newBalance)
{
    int userId = getUserId(username);
    if (userId <= 0) {
        qDebug() << "用户不存在:" << username;
        return false;
    }

    QSqlDatabase::database().transaction();

    try {
        // 1. 查询当前余额
        QSqlQuery query;
        query.prepare("SELECT balance FROM wallets WHERE user_id = :user_id");
        query.bindValue(":user_id", userId);

        if (!query.exec() || !query.next()) {
            QSqlDatabase::database().rollback();
            return false;
        }

        double oldBalance = query.value(0).toDouble();

        // 2. 检查余额是否足够
        if (oldBalance < amount) {
            qDebug() << "余额不足: 需要" << amount << "，当前" << oldBalance;
            QSqlDatabase::database().rollback();
            return false;
        }

        newBalance = oldBalance - amount;

        // 3. 更新余额
        query.prepare("UPDATE wallets SET balance = :new_balance WHERE user_id = :user_id");
        query.bindValue(":new_balance", newBalance);
        query.bindValue(":user_id", userId);

        if (!query.exec()) {
            QSqlDatabase::database().rollback();
            return false;
        }

        QSqlDatabase::database().commit();
        return true;

    } catch (...) {
        QSqlDatabase::database().rollback();
        return false;
    }
}

bool DatabaseManager::getFlightAvailableSeats(int flightId, int &availableSeats)
{
    QSqlQuery query;
    query.prepare("SELECT available_seats FROM flights WHERE id = ?");
    query.addBindValue(flightId);

    if (query.exec() && query.next()) {
        availableSeats = query.value(0).toInt();
        return true;
    }
    return false;
}

bool DatabaseManager::getCabinAvailableSeats(int cabinId, int &availableSeats)
{
    QSqlQuery query;
    query.prepare("SELECT available_seats FROM cabins WHERE id = ?");
    query.addBindValue(cabinId);

    if (query.exec() && query.next()) {
        availableSeats = query.value(0).toInt();
        return true;
    }
    return false;
}
