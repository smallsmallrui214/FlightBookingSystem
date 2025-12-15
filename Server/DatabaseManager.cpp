#include "databasemanager.h"
#include <QThread>
#include <QCoreApplication>
#include <functional>
#include <QJsonArray>
#include <QJsonObject>

// 静态成员初始化
int DatabaseManager::connectionCounter = 0;

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
{
    qDebug() << "DatabaseManager 初始化";
}

DatabaseManager::~DatabaseManager()
{
    qDebug() << "DatabaseManager 析构，清理连接";

    // 等待所有操作完成
    QThread::msleep(100);

    // 清理所有数据库连接
    QStringList connections = QSqlDatabase::connectionNames();
    qDebug() << "需要清理的连接数:" << connections.size();

    for (const QString &name : connections) {
        qDebug() << "清理连接:" << name;
        if (QSqlDatabase::contains(name)) {
            QSqlDatabase db = QSqlDatabase::database(name, false);
            if (db.isOpen()) {
                db.close();
            }
            // 等待数据库操作完成
            QThread::msleep(20);
            QSqlDatabase::removeDatabase(name);
        }
    }
}

DatabaseManager& DatabaseManager::getInstance()
{
    static DatabaseManager instance;
    return instance;
}

bool DatabaseManager::initializeDatabase()
{
    qDebug() << "初始化数据库连接";

    // 检查是否已经有默认连接
    if (QSqlDatabase::contains("flight_main_connection")) {
        QSqlDatabase db = QSqlDatabase::database("flight_main_connection");
        if (db.isOpen()) {
            qDebug() << "默认连接已存在且已打开";
            return true;
        }
    }

    // 创建新的默认连接
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC", "flight_main_connection");
    db.setDatabaseName("flight_booking_system");
    db.setUserName("root");
    db.setPassword("123Xyf");

    // 设置连接选项
    db.setConnectOptions("SQL_ATTR_CONNECTION_POOLING=SQL_CP_ONE_PER_HENV;"
                         "SQL_ATTR_ODBC_VERSION=SQL_OV_ODBC3;"
                         "SQL_ATTR_ACCESS_MODE=SQL_MODE_READ_WRITE;"
                         "SQL_ATTR_AUTOCOMMIT=SQL_AUTOCOMMIT_ON;");

    if (!db.open()) {
        qDebug() << "数据库默认连接失败:" << db.lastError().text();
        QSqlDatabase::removeDatabase("flight_main_connection");
        return false;
    }

    qDebug() << "数据库默认连接成功";
    return true;
}

QSqlDatabase DatabaseManager::getNewConnection()
{
    QMutexLocker locker(&connectionMutex);

    // 生成唯一的连接名
    QString connectionName = QString("flight_conn_%1_%2")
                                 .arg(QDateTime::currentMSecsSinceEpoch())
                                 .arg(++connectionCounter);

    qDebug() << "创建新连接:" << connectionName;

    // 添加数据库驱动
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC", connectionName);
    if (!db.isValid()) {
        qDebug() << "创建数据库连接失败: 驱动无效";
        return QSqlDatabase();
    }

    db.setDatabaseName("flight_booking_system");
    db.setUserName("root");
    db.setPassword("123Xyf");

    // 设置连接选项以避免序列错误
    db.setConnectOptions("SQL_ATTR_CONNECTION_POOLING=SQL_CP_ONE_PER_HENV;"
                         "SQL_ATTR_ODBC_VERSION=SQL_OV_ODBC3;"
                         "SQL_ATTR_ACCESS_MODE=SQL_MODE_READ_WRITE;"
                         "SQL_ATTR_AUTOCOMMIT=SQL_AUTOCOMMIT_ON;");

    if (!db.open()) {
        qDebug() << "打开数据库连接失败:" << db.lastError().text();
        // 注意：这里不要立即移除连接，让调用者处理
        return QSqlDatabase();
    }

    qDebug() << "数据库连接成功:" << connectionName;
    return db;
}

void DatabaseManager::safeCloseConnection(QSqlDatabase &db)
{
    if (db.isValid()) {
        QString connectionName = db.connectionName();

        // 检查连接是否仍然有效
        if (QSqlDatabase::contains(connectionName)) {
            // 先关闭连接
            if (db.isOpen()) {
                db.close();
            }

            // 等待一小段时间确保所有查询完成
            QThread::msleep(20);

            // 清理任何挂起的操作
            QCoreApplication::processEvents();

            // 移除连接
            if (QSqlDatabase::contains(connectionName)) {
                QSqlDatabase::removeDatabase(connectionName);
                qDebug() << "安全关闭连接:" << connectionName;
            }
        }
    }
}

bool DatabaseManager::executeWithTransaction(QSqlDatabase &db, std::function<bool(QSqlDatabase&)> operation)
{
    if (!db.isValid() || !db.isOpen()) {
        qDebug() << "数据库连接无效或未打开";
        return false;
    }

    QSqlQuery query(db);
    if (!db.transaction()) {
        qDebug() << "开始事务失败:" << db.lastError().text();
        return false;
    }

    bool success = false;
    try {
        success = operation(db);

        if (success) {
            if (!db.commit()) {
                qDebug() << "提交事务失败:" << db.lastError().text();
                db.rollback();
                return false;
            }
        } else {
            db.rollback();
            qDebug() << "操作失败，回滚事务";
        }
    } catch (const std::exception &e) {
        db.rollback();
        qDebug() << "事务执行过程中发生异常:" << e.what();
        return false;
    } catch (...) {
        db.rollback();
        qDebug() << "事务执行过程中发生未知异常";
        return false;
    }

    return success;
}

bool DatabaseManager::validateUser(const QString &username, const QString &password)
{
    QSqlDatabase db = getNewConnection();
    if (!db.isValid() || !db.isOpen()) {
        qDebug() << "无法获取数据库连接";
        return false;
    }

    bool isValid = false;
    try {
        QSqlQuery query(db);
        query.prepare("SELECT id FROM users WHERE username = ? AND password = ?");
        query.addBindValue(username);
        query.addBindValue(password);

        if (query.exec()) {
            isValid = query.next();
            if (isValid) {
                qDebug() << "用户验证成功:" << username;
            } else {
                qDebug() << "用户验证失败: 用户名或密码错误";
            }
        } else {
            qDebug() << "用户验证查询失败:" << query.lastError().text();
        }

        query.finish();
    } catch (...) {
        qDebug() << "用户验证过程中发生异常";
    }

    safeCloseConnection(db);
    return isValid;
}

bool DatabaseManager::createUser(const QString &username, const QString &password, const QString &email)
{
    if (userExists(username)) {
        qDebug() << "创建用户失败: 用户名" << username << "已存在";
        return false;
    }

    QSqlDatabase db = getNewConnection();
    if (!db.isValid() || !db.isOpen()) {
        qDebug() << "无法获取数据库连接";
        return false;
    }

    bool success = executeWithTransaction(db, [&](QSqlDatabase &db) -> bool {
        // 1. 创建用户
        QSqlQuery query(db);
        query.prepare("INSERT INTO users (username, password, email, created_at) "
                      "VALUES (?, ?, ?, NOW())");
        query.addBindValue(username);
        query.addBindValue(password);
        query.addBindValue(email);

        if (!query.exec()) {
            qDebug() << "创建用户失败:" << query.lastError().text();
            return false;
        }

        int userId = query.lastInsertId().toInt();
        if (userId <= 0) {
            qDebug() << "获取用户ID失败";
            return false;
        }

        qDebug() << "用户创建成功，用户ID:" << userId;

        // 2. 创建钱包
        query.prepare("INSERT INTO wallets (user_id, balance, created_at) "
                      "VALUES (?, 0.0, NOW())");
        query.addBindValue(userId);

        if (!query.exec()) {
            qDebug() << "创建钱包失败:" << query.lastError().text();
            return false;
        }

        qDebug() << "钱包创建成功，用户ID:" << userId;
        return true;
    });

    safeCloseConnection(db);
    return success;
}

bool DatabaseManager::userExists(const QString &username)
{
    QSqlDatabase db = getNewConnection();
    if (!db.isValid() || !db.isOpen()) {
        qDebug() << "无法获取数据库连接";
        return false;
    }

    bool exists = false;
    try {
        QSqlQuery query(db);
        query.prepare("SELECT COUNT(*) FROM users WHERE username = ?");
        query.addBindValue(username);

        if (query.exec() && query.next()) {
            exists = query.value(0).toInt() > 0;
        }

        query.finish();
    } catch (...) {
        qDebug() << "检查用户存在性时发生异常";
    }

    safeCloseConnection(db);
    return exists;
}

int DatabaseManager::getUserId(const QString &username)
{
    QSqlDatabase db = getNewConnection();
    if (!db.isValid() || !db.isOpen()) {
        qDebug() << "无法获取数据库连接";
        return -1;
    }

    int userId = -1;
    try {
        QSqlQuery query(db);
        query.prepare("SELECT id FROM users WHERE username = ?");
        query.addBindValue(username);

        if (query.exec() && query.next()) {
            userId = query.value(0).toInt();
        }

        query.finish();
    } catch (...) {
        qDebug() << "获取用户ID时发生异常";
    }

    safeCloseConnection(db);
    return userId;
}

bool DatabaseManager::updateLastLogin(const QString &username)
{
    QSqlDatabase db = getNewConnection();
    if (!db.isValid() || !db.isOpen()) {
        qDebug() << "无法获取数据库连接";
        return false;
    }

    bool success = false;
    try {
        QSqlQuery query(db);
        query.prepare("UPDATE users SET last_login = NOW() WHERE username = ?");
        query.addBindValue(username);

        success = query.exec();
        if (success) {
            qDebug() << "更新最后登录时间成功:" << username;
        } else {
            qDebug() << "更新最后登录时间失败:" << query.lastError().text();
        }

        query.finish();
    } catch (...) {
        qDebug() << "更新最后登录时间时发生异常";
    }

    safeCloseConnection(db);
    return success;
}

bool DatabaseManager::queryUserBalance(const QString &username, double &balance)
{
    QSqlDatabase db = getNewConnection();
    if (!db.isValid() || !db.isOpen()) {
        qDebug() << "无法获取数据库连接";
        return false;
    }

    bool success = false;
    try {
        // 1. 获取用户ID
        QSqlQuery userIdQuery(db);
        userIdQuery.prepare("SELECT id FROM users WHERE username = ?");
        userIdQuery.addBindValue(username);

        if (!userIdQuery.exec() || !userIdQuery.next()) {
            qDebug() << "查询余额失败：用户不存在:" << username;
            userIdQuery.finish();
            safeCloseConnection(db);
            return false;
        }

        int userId = userIdQuery.value(0).toInt();
        userIdQuery.finish();

        // 2. 查询余额
        QSqlQuery balanceQuery(db);
        balanceQuery.prepare("SELECT balance FROM wallets WHERE user_id = ?");
        balanceQuery.addBindValue(userId);

        if (balanceQuery.exec() && balanceQuery.next()) {
            balance = balanceQuery.value(0).toDouble();
            qDebug() << "用户" << username << "余额查询成功:" << balance;
            success = true;
        } else {
            qDebug() << "用户" << username << "钱包不存在，将创建新钱包";

            // 3. 创建钱包
            QSqlQuery createQuery(db);
            createQuery.prepare("INSERT INTO wallets (user_id, balance, created_at) "
                                "VALUES (?, 0.0, NOW())");
            createQuery.addBindValue(userId);

            if (createQuery.exec()) {
                balance = 0.0;
                qDebug() << "钱包创建成功，初始余额: 0.0";
                success = true;
            } else {
                qDebug() << "创建钱包失败:" << createQuery.lastError().text();
                success = false;
            }
            createQuery.finish();
        }
        balanceQuery.finish();

    } catch (...) {
        qDebug() << "查询用户余额时发生异常";
        success = false;
    }

    safeCloseConnection(db);
    return success;
}

bool DatabaseManager::rechargeUserWallet(const QString &username, double amount, double &newBalance)
{
    if (amount <= 0) {
        qDebug() << "充值金额必须大于0";
        return false;
    }

    QSqlDatabase db = getNewConnection();
    if (!db.isValid() || !db.isOpen()) {
        qDebug() << "无法获取数据库连接";
        return false;
    }

    bool success = executeWithTransaction(db, [&](QSqlDatabase &db) -> bool {
        // 1. 获取用户ID
        QSqlQuery userIdQuery(db);
        userIdQuery.prepare("SELECT id FROM users WHERE username = ?");
        userIdQuery.addBindValue(username);

        if (!userIdQuery.exec() || !userIdQuery.next()) {
            qDebug() << "充值失败：用户不存在:" << username;
            return false;
        }

        int userId = userIdQuery.value(0).toInt();
        userIdQuery.finish();

        // 2. 获取当前余额
        QSqlQuery oldBalanceQuery(db);
        oldBalanceQuery.prepare("SELECT balance FROM wallets WHERE user_id = ?");
        oldBalanceQuery.addBindValue(userId);

        double oldBalance = 0.0;
        if (oldBalanceQuery.exec() && oldBalanceQuery.next()) {
            oldBalance = oldBalanceQuery.value(0).toDouble();
        } else {
            // 钱包不存在，创建钱包
            QSqlQuery createQuery(db);
            createQuery.prepare("INSERT INTO wallets (user_id, balance, created_at) "
                                "VALUES (?, 0.0, NOW())");
            createQuery.addBindValue(userId);

            if (!createQuery.exec()) {
                qDebug() << "创建钱包失败:" << createQuery.lastError().text();
                return false;
            }
            oldBalance = 0.0;
            createQuery.finish();
        }
        oldBalanceQuery.finish();

        // 3. 更新余额
        QSqlQuery updateQuery(db);
        updateQuery.prepare("UPDATE wallets SET balance = balance + ? WHERE user_id = ?");
        updateQuery.addBindValue(amount);
        updateQuery.addBindValue(userId);

        if (!updateQuery.exec()) {
            qDebug() << "更新余额失败:" << updateQuery.lastError().text();
            return false;
        }
        updateQuery.finish();

        // 4. 获取新余额
        QSqlQuery newBalanceQuery(db);
        newBalanceQuery.prepare("SELECT balance FROM wallets WHERE user_id = ?");
        newBalanceQuery.addBindValue(userId);

        if (!newBalanceQuery.exec() || !newBalanceQuery.next()) {
            qDebug() << "获取新余额失败";
            return false;
        }

        newBalance = newBalanceQuery.value(0).toDouble();
        newBalanceQuery.finish();

        // 5. 记录充值历史
        QSqlQuery recordQuery(db);
        recordQuery.prepare("INSERT INTO recharge_records (user_id, amount, old_balance, new_balance, recharge_time) "
                            "VALUES (?, ?, ?, ?, NOW())");
        recordQuery.addBindValue(userId);
        recordQuery.addBindValue(amount);
        recordQuery.addBindValue(oldBalance);
        recordQuery.addBindValue(newBalance);

        if (!recordQuery.exec()) {
            qDebug() << "记录充值历史失败:" << recordQuery.lastError().text();
            // 不返回失败，因为充值本身已经成功
        }
        recordQuery.finish();

        qDebug() << "充值成功: 用户" << username
                 << "，金额:" << amount
                 << "，旧余额:" << oldBalance
                 << "，新余额:" << newBalance;

        return true;
    });

    safeCloseConnection(db);
    return success;
}

QJsonArray DatabaseManager::getRechargeRecords(const QString &username)
{
    QJsonArray recordsArray;

    QSqlDatabase db = getNewConnection();
    if (!db.isValid() || !db.isOpen()) {
        qDebug() << "无法获取数据库连接";
        return recordsArray;
    }

    try {
        // 1. 获取用户ID
        int userId = getUserId(username);
        if (userId <= 0) {
            qDebug() << "用户不存在:" << username;
            safeCloseConnection(db);
            return recordsArray;
        }

        // 2. 查询充值记录
        QSqlQuery query(db);
        query.prepare("SELECT amount, old_balance, new_balance, recharge_time "
                      "FROM recharge_records "
                      "WHERE user_id = ? "
                      "ORDER BY recharge_time DESC");
        query.addBindValue(userId);

        if (query.exec()) {
            while (query.next()) {
                QJsonObject recordObj;

                double amount = query.value("amount").toDouble();
                double oldBalance = query.value("old_balance").toDouble();
                double newBalance = query.value("new_balance").toDouble();
                QDateTime rechargeTime = query.value("recharge_time").toDateTime();

                recordObj["amount"] = amount;
                recordObj["before_balance"] = oldBalance;
                recordObj["after_balance"] = newBalance;
                recordObj["recharge_time"] = rechargeTime.toString("yyyy-MM-dd HH:mm:ss");

                recordsArray.append(recordObj);
            }

            qDebug() << "用户" << username << "充值记录查询成功，找到" << recordsArray.size() << "条记录";
        } else {
            qDebug() << "查询充值记录失败:" << query.lastError().text();
        }

        query.finish();
    } catch (...) {
        qDebug() << "查询充值记录时发生异常";
    }

    safeCloseConnection(db);
    return recordsArray;
}

bool DatabaseManager::deductFromWallet(const QString &username, double amount, double &newBalance)
{
    QSqlDatabase db = getNewConnection();
    if (!db.isValid() || !db.isOpen()) {
        qDebug() << "无法获取数据库连接";
        return false;
    }

    bool success = executeWithTransaction(db, [&](QSqlDatabase &db) -> bool {
        // 1. 获取用户ID
        QSqlQuery userIdQuery(db);
        userIdQuery.prepare("SELECT id FROM users WHERE username = ?");
        userIdQuery.addBindValue(username);

        if (!userIdQuery.exec() || !userIdQuery.next()) {
            qDebug() << "扣款失败：用户不存在:" << username;
            return false;
        }

        int userId = userIdQuery.value(0).toInt();
        userIdQuery.finish();

        // 2. 查询当前余额
        QSqlQuery balanceQuery(db);
        balanceQuery.prepare("SELECT balance FROM wallets WHERE user_id = ?");
        balanceQuery.addBindValue(userId);

        if (!balanceQuery.exec() || !balanceQuery.next()) {
            qDebug() << "查询余额失败:" << balanceQuery.lastError().text();
            return false;
        }

        double oldBalance = balanceQuery.value(0).toDouble();
        balanceQuery.finish();

        // 3. 检查余额是否足够
        if (oldBalance < amount) {
            qDebug() << "余额不足: 需要" << amount << "，当前" << oldBalance;
            return false;
        }

        newBalance = oldBalance - amount;

        // 4. 更新余额
        QSqlQuery updateQuery(db);
        updateQuery.prepare("UPDATE wallets SET balance = ? WHERE user_id = ?");
        updateQuery.addBindValue(newBalance);
        updateQuery.addBindValue(userId);

        if (!updateQuery.exec()) {
            qDebug() << "更新余额失败:" << updateQuery.lastError().text();
            return false;
        }
        updateQuery.finish();

        qDebug() << "扣款成功: 用户" << username
                 << "，扣款金额:" << amount
                 << "，旧余额:" << oldBalance
                 << "，新余额:" << newBalance;

        return true;
    });

    safeCloseConnection(db);
    return success;
}

bool DatabaseManager::processBooking(const QString &username, int flightId, int cabinId,
                                     const QString &passengerName, const QString &passengerId,
                                     const QString &passengerPhone, double totalPrice,
                                     QString &bookingNumber, double &newBalance)
{
    QSqlDatabase db = getNewConnection();
    if (!db.isValid() || !db.isOpen()) {
        qDebug() << "无法获取数据库连接";
        return false;
    }

    bool success = executeWithTransaction(db, [&](QSqlDatabase &db) -> bool {
        // 1. 获取用户ID
        QSqlQuery userIdQuery(db);
        userIdQuery.prepare("SELECT id FROM users WHERE username = ?");
        userIdQuery.addBindValue(username);

        if (!userIdQuery.exec() || !userIdQuery.next()) {
            qDebug() << "预订失败：用户不存在:" << username;
            return false;
        }

        int userId = userIdQuery.value(0).toInt();
        userIdQuery.finish();

        // 2. 检查余额
        QSqlQuery balanceQuery(db);
        balanceQuery.prepare("SELECT balance FROM wallets WHERE user_id = ?");
        balanceQuery.addBindValue(userId);

        double currentBalance = 0.0;
        if (!balanceQuery.exec() || !balanceQuery.next()) {
            qDebug() << "查询余额失败";
            return false;
        }
        currentBalance = balanceQuery.value(0).toDouble();
        balanceQuery.finish();

        if (currentBalance < totalPrice) {
            qDebug() << "余额不足: 需要" << totalPrice << "，当前" << currentBalance;
            return false;
        }

        // 3. 检查座位可用性
        QSqlQuery flightSeatQuery(db);
        flightSeatQuery.prepare("SELECT available_seats FROM flights WHERE id = ?");
        flightSeatQuery.addBindValue(flightId);

        if (!flightSeatQuery.exec() || !flightSeatQuery.next()) {
            qDebug() << "查询航班座位失败";
            return false;
        }

        int flightSeats = flightSeatQuery.value(0).toInt();
        flightSeatQuery.finish();

        if (flightSeats <= 0) {
            qDebug() << "航班座位已售罄";
            return false;
        }

        QSqlQuery cabinSeatQuery(db);
        cabinSeatQuery.prepare("SELECT available_seats FROM cabins WHERE id = ?");
        cabinSeatQuery.addBindValue(cabinId);

        if (!cabinSeatQuery.exec() || !cabinSeatQuery.next()) {
            qDebug() << "查询舱位座位失败";
            return false;
        }

        int cabinSeats = cabinSeatQuery.value(0).toInt();
        cabinSeatQuery.finish();

        if (cabinSeats <= 0) {
            qDebug() << "舱位座位已售罄";
            return false;
        }

        // 4. 扣款
        newBalance = currentBalance - totalPrice;
        QSqlQuery deductQuery(db);
        deductQuery.prepare("UPDATE wallets SET balance = ? WHERE user_id = ?");
        deductQuery.addBindValue(newBalance);
        deductQuery.addBindValue(userId);

        if (!deductQuery.exec()) {
            qDebug() << "扣款失败:" << deductQuery.lastError().text();
            return false;
        }
        deductQuery.finish();

        // 5. 更新座位数
        QSqlQuery updateFlightQuery(db);
        updateFlightQuery.prepare("UPDATE flights SET available_seats = available_seats - 1 WHERE id = ?");
        updateFlightQuery.addBindValue(flightId);

        if (!updateFlightQuery.exec()) {
            qDebug() << "更新航班座位失败:" << updateFlightQuery.lastError().text();
            return false;
        }
        updateFlightQuery.finish();

        QSqlQuery updateCabinQuery(db);
        updateCabinQuery.prepare("UPDATE cabins SET available_seats = available_seats - 1 WHERE id = ?");
        updateCabinQuery.addBindValue(cabinId);

        if (!updateCabinQuery.exec()) {
            qDebug() << "更新舱位座位失败:" << updateCabinQuery.lastError().text();
            return false;
        }
        updateCabinQuery.finish();

        // 6. 创建预订记录
        QSqlQuery bookingQuery(db);
        bookingQuery.prepare("INSERT INTO bookings (user_id, flight_id, cabin_id, passenger_name, "
                             "passenger_id, passenger_phone, total_price, booking_time, status) "
                             "VALUES (?, ?, ?, ?, ?, ?, ?, NOW(), '已预订')");
        bookingQuery.addBindValue(userId);
        bookingQuery.addBindValue(flightId);
        bookingQuery.addBindValue(cabinId);
        bookingQuery.addBindValue(passengerName);
        bookingQuery.addBindValue(passengerId);
        bookingQuery.addBindValue(passengerPhone);
        bookingQuery.addBindValue(totalPrice);

        if (!bookingQuery.exec()) {
            qDebug() << "创建预订记录失败:" << bookingQuery.lastError().text();
            return false;
        }

        bookingNumber = QString::number(bookingQuery.lastInsertId().toInt());
        bookingQuery.finish();

        qDebug() << "预订成功: 用户" << username
                 << "，订单号:" << bookingNumber
                 << "，金额:" << totalPrice
                 << "，新余额:" << newBalance;

        return true;
    });

    safeCloseConnection(db);
    return success;
}

bool DatabaseManager::getFlightAvailableSeats(int flightId, int &availableSeats)
{
    QSqlDatabase db = getNewConnection();
    if (!db.isValid() || !db.isOpen()) {
        qDebug() << "无法获取数据库连接";
        return false;
    }

    bool success = false;
    try {
        QSqlQuery query(db);
        query.prepare("SELECT available_seats FROM flights WHERE id = ?");
        query.addBindValue(flightId);

        if (query.exec() && query.next()) {
            availableSeats = query.value(0).toInt();
            success = true;
        }

        query.finish();
    } catch (...) {
        qDebug() << "查询航班座位时发生异常";
    }

    safeCloseConnection(db);
    return success;
}

bool DatabaseManager::getCabinAvailableSeats(int cabinId, int &availableSeats)
{
    QSqlDatabase db = getNewConnection();
    if (!db.isValid() || !db.isOpen()) {
        qDebug() << "无法获取数据库连接";
        return false;
    }

    bool success = false;
    try {
        QSqlQuery query(db);
        query.prepare("SELECT available_seats FROM cabins WHERE id = ?");
        query.addBindValue(cabinId);

        if (query.exec() && query.next()) {
            availableSeats = query.value(0).toInt();
            success = true;
        }

        query.finish();
    } catch (...) {
        qDebug() << "查询舱位座位时发生异常";
    }

    safeCloseConnection(db);
    return success;
}
