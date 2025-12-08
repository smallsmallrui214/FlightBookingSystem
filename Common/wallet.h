#ifndef WALLET_H
#define WALLET_H

#include <QString>
#include <QJsonObject>
#include <QDateTime>

class Wallet
{
public:
    Wallet();
    Wallet(int userId, double balance);
    Wallet(int userId, double balance, const QDateTime &lastUpdated);

    QJsonObject toJson() const;
    static Wallet fromJson(const QJsonObject &json);

    // Getters
    int getId() const { return id; }
    int getUserId() const { return userId; }
    double getBalance() const { return balance; }
    QDateTime getLastUpdated() const { return lastUpdated; }

    // Setters
    void setId(int id) { this->id = id; }
    void setUserId(int userId) { this->userId = userId; }
    void setBalance(double balance) { this->balance = balance; }
    void setLastUpdated(const QDateTime &time) { this->lastUpdated = time; }

private:
    int id;
    int userId;
    double balance;
    QDateTime lastUpdated;
};

#endif // WALLET_H
