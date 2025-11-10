#ifndef USER_H
#define USER_H

#include <QString>
#include <QDateTime>

class User
{
public:
    User();
    User(int id, const QString &username, const QString &email, const QDateTime &createdAt);

    int getId() const { return id; }
    QString getUsername() const { return username; }
    QString getEmail() const { return email; }
    QDateTime getCreatedAt() const { return createdAt; }
    QDateTime getLastLogin() const { return lastLogin; }

    void setLastLogin(const QDateTime &time) { lastLogin = time; }

private:
    int id;
    QString username;
    QString email;
    QDateTime createdAt;
    QDateTime lastLogin;
};

#endif // USER_H
