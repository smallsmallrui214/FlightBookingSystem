#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QString>
#include <QJsonObject>
#include <QJsonDocument>

// 消息类型枚举
enum MessageType {
    TEST_MESSAGE = 0,          // 测试消息
    LOGIN_REQUEST = 100,       // 登录请求
    LOGIN_RESPONSE = 101,      // 登录响应
    REGISTER_REQUEST = 102,    // 注册请求
    REGISTER_RESPONSE = 103,   // 注册响应

    // 用户验证
    CHECK_USERNAME_REQUEST = 0x1005,  // 新增：检查用户名是否存在
    CHECK_USERNAME_RESPONSE = 0x1006, // 新增：检查用户名响应

    // 航班查询相关
    FLIGHT_SEARCH_REQUEST = 200,
    FLIGHT_SEARCH_RESPONSE = 201,
    FLIGHT_DETAIL_REQUEST = 202,
    FLIGHT_DETAIL_RESPONSE = 203,

    // 订单相关
    BOOKING_REQUEST = 300,
    BOOKING_RESPONSE = 301
};

// 基础消息结构
struct NetworkMessage {
    MessageType type;
    QJsonObject data;

    // 序列化为JSON字符串
    QByteArray toJson() const {
        QJsonObject obj;
        obj["type"] = static_cast<int>(type);
        obj["data"] = data;
        return QJsonDocument(obj).toJson();
    }

    // 从JSON字符串反序列化
    static NetworkMessage fromJson(const QByteArray &json) {
        NetworkMessage msg;
        QJsonDocument doc = QJsonDocument::fromJson(json);
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            msg.type = static_cast<MessageType>(obj["type"].toInt());
            msg.data = obj["data"].toObject();
        }
        return msg;
    }
};

#endif // PROTOCOL_H
