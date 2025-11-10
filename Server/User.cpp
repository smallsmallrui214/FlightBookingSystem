#include "user.h"

User::User()
    : id(-1)
{
}

User::User(int id, const QString &username, const QString &email, const QDateTime &createdAt)
    : id(id), username(username), email(email), createdAt(createdAt)
{
}
