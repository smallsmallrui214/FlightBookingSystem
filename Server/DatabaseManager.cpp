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
