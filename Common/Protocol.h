#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QJsonObject>
#include <QJsonDocument>
#include <QByteArray>

// 网络消息结构体
struct NetworkMessage
{
    int type;           // 消息类型
    QJsonObject data;   // 消息数据

    QByteArray toJson() const {
        QJsonObject obj;
        obj["type"] = type;
        obj["data"] = data;
        QJsonDocument doc(obj);
        return doc.toJson();
    }

    static NetworkMessage fromJson(const QByteArray &json) {
        NetworkMessage msg;
        QJsonDocument doc = QJsonDocument::fromJson(json);
        if (!doc.isNull()) {
            QJsonObject obj = doc.object();
            msg.type = obj["type"].toInt();
            msg.data = obj["data"].toObject();
        }
        return msg;
    }
};

// 消息类型枚举
enum MessageType {
    // 连接测试
    CONNECTION_TEST = 0,
    CONNECTION_TEST_RESPONSE = 1,

    // 用户认证相关
    LOGIN_REQUEST = 2,
    LOGIN_RESPONSE = 3,
    REGISTER_REQUEST = 4,
    REGISTER_RESPONSE = 5,
    LOGOUT_REQUEST = 6,
    LOGOUT_RESPONSE = 7,
    CHECK_USERNAME_REQUEST = 8,
    CHECK_USERNAME_RESPONSE = 9,

    // 用户信息修改
    CHANGE_USERNAME_REQUEST = 10,
    CHANGE_USERNAME_RESPONSE = 11,
    CHANGE_PASSWORD_REQUEST = 12,
    CHANGE_PASSWORD_RESPONSE = 13,

    // 航班查询相关
    FLIGHT_SEARCH_REQUEST = 20,
    FLIGHT_SEARCH_RESPONSE = 21,
    FLIGHT_DETAIL_REQUEST = 22,
    FLIGHT_DETAIL_RESPONSE = 23,
    CABIN_SEARCH_REQUEST = 24,
    CABIN_SEARCH_RESPONSE = 25,

    // 预订相关
    BOOKING_REQUEST = 30,
    BOOKING_RESPONSE = 31,
    ORDER_LIST_REQUEST = 32,
    ORDER_LIST_RESPONSE = 33,
    ORDER_DETAIL_REQUEST = 34,
    ORDER_DETAIL_RESPONSE = 35,
    ORDER_CANCEL_REQUEST = 36,
    ORDER_CANCEL_RESPONSE = 37,

    // 钱包相关
    WALLET_QUERY_REQUEST = 40,
    WALLET_QUERY_RESPONSE = 41,
    RECHARGE_REQUEST = 42,
    RECHARGE_RESPONSE = 43,
    RECHARGE_RECORDS_REQUEST = 44,
    RECHARGE_RECORDS_RESPONSE = 45,

    // 系统管理相关
    SYSTEM_STATUS_REQUEST = 90,
    SYSTEM_STATUS_RESPONSE = 91
};

#endif // PROTOCOL_H
