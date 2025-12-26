#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QString>
#include <QJsonObject>
#include <QJsonDocument>
#include <QByteArray>

// 消息类型枚举
enum MessageType {
    TEST_MESSAGE = 0,          // 测试消息
    LOGIN_REQUEST = 100,       // 登录请求
    LOGIN_RESPONSE = 101,      // 登录响应
    REGISTER_REQUEST = 102,    // 注册请求
    REGISTER_RESPONSE = 103,   // 注册响应

    // 用户验证
    CHECK_USERNAME_REQUEST = 0x1005,  // 检查用户名是否存在
    CHECK_USERNAME_RESPONSE = 0x1006, // 检查用户名响应

    // 用户信息修改 (新增)
    CHANGE_USERNAME_REQUEST = 104,    // 修改用户名请求
    CHANGE_USERNAME_RESPONSE = 105,   // 修改用户名响应
    CHANGE_PASSWORD_REQUEST = 106,    // 修改密码请求
    CHANGE_PASSWORD_RESPONSE = 107,   // 修改密码响应

    // 航班查询相关
    FLIGHT_SEARCH_REQUEST = 200,
    FLIGHT_SEARCH_RESPONSE = 201,
    FLIGHT_DETAIL_REQUEST = 202,
    FLIGHT_DETAIL_RESPONSE = 203,

    // 舱位查询相关
    CABIN_SEARCH_REQUEST = 204,
    CABIN_SEARCH_RESPONSE = 205,

    // 预订和订单相关 (300-399)
    BOOKING_REQUEST = 300,         // 预订请求
    BOOKING_RESPONSE = 301,        // 预订响应
    ORDER_LIST_REQUEST = 302,      // 订单列表请求
    ORDER_LIST_RESPONSE = 303,     // 订单列表响应
    ORDER_CANCEL_REQUEST = 304,    // 取消订单请求
    ORDER_CANCEL_RESPONSE = 305,   // 取消订单响应

    // 钱包和支付相关 (400-499)
    WALLET_QUERY_REQUEST = 400,    // 钱包查询请求
    WALLET_QUERY_RESPONSE = 401,   // 钱包查询响应
    RECHARGE_REQUEST = 402,        // 充值请求
    RECHARGE_RESPONSE = 403,       // 充值响应
    PAYMENT_REQUEST = 404,         // 支付请求（用于预订时扣款）
    PAYMENT_RESPONSE = 405,        // 支付响应
    RECHARGE_RECORDS_REQUEST = 24,      // 充值记录查询请求
    RECHARGE_RECORDS_RESPONSE = 25,     // 充值记录查询响应
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
