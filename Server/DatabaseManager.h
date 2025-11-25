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

private:
    DatabaseManager(QObject *parent = nullptr);
    bool openDatabase();
};

#endif // DATABASEMANAGER_H
