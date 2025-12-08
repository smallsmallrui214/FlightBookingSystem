#include "wallet.h"
#include <QJsonObject>

Wallet::Wallet()
    : id(-1), userId(-1), balance(0.0), lastUpdated(QDateTime::currentDateTime())
{
}

Wallet::Wallet(int userId, double balance)
    : id(-1), userId(userId), balance(balance), lastUpdated(QDateTime::currentDateTime())
{
}

Wallet::Wallet(int userId, double balance, const QDateTime &lastUpdated)
    : id(-1), userId(userId), balance(balance), lastUpdated(lastUpdated)
{
}

QJsonObject Wallet::toJson() const
{
    QJsonObject obj;
    obj["id"] = id;
    obj["user_id"] = userId;
    obj["balance"] = balance;
    obj["last_updated"] = lastUpdated.toString(Qt::ISODate);
    return obj;
}

Wallet Wallet::fromJson(const QJsonObject &json)
{
    Wallet wallet;
    wallet.id = json["id"].toInt();
    wallet.userId = json["user_id"].toInt();
    wallet.balance = json["balance"].toDouble();
    wallet.lastUpdated = QDateTime::fromString(json["last_updated"].toString(), Qt::ISODate);
    return wallet;
}
