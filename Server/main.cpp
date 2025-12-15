#include <QCoreApplication>
#include <QDebug>
#include <QJsonArray>
#include <QRandomGenerator>
#include <QTime>
#include <QDateTime>
#include <cstdlib>
#include <ctime>
#include "servernetworkmanager.h"
#include "databasemanager.h"
#include "../Common/flight.h"
#include "../Common/cabin.h"
#include "../Common/booking.h"
#include "../Common/protocol.h"

class TestServer : public QObject
{
    Q_OBJECT

public:
    TestServer() {
        // 初始化随机数种子
        srand(QTime::currentTime().msec());
        srand(static_cast<unsigned int>(time(nullptr)));

        // 初始化数据库连接
        if (!DatabaseManager::getInstance().initializeDatabase()) {
            qDebug() << "数据库连接失败!";
            return;
        }

        connect(&networkManager, &ServerNetworkManager::messageReceived,
                this, &TestServer::onMessageReceived);
        connect(&networkManager, &ServerNetworkManager::clientConnected,
                this, &TestServer::onClientConnected);
        connect(&networkManager, &ServerNetworkManager::clientDisconnected,
                this, &TestServer::onClientDisconnected);
    }

    void start() {
        if (networkManager.startServer(8888)) {
            qDebug() << "=== 航班服务器已启动 ===";
            qDebug() << "端口: 8888";
            qDebug() << "等待客户端连接...";
        }
    }

private slots:
    void onClientConnected(QTcpSocket *client) {
        qDebug() << "客户端连接:" << client->peerAddress().toString();

        NetworkMessage welcomeMsg;
        welcomeMsg.type = CONNECTION_TEST_RESPONSE;
        welcomeMsg.data["content"] = "欢迎连接到航班服务器!";
        networkManager.sendMessage(welcomeMsg, client);
    }

    void onClientDisconnected(QTcpSocket *client) {
        // 清理登录状态
        if (socketToUser.contains(client)) {
            QString username = socketToUser[client];
            loggedInUsers.remove(username);
            socketToUser.remove(client);
            qDebug() << "用户" << username << "断开连接，清理登录状态";
        }
    }

    void debugNetworkMessage(const NetworkMessage &message, const QString &requestName) {
        qDebug() << "=== 调试：" << requestName << "===";
        qDebug() << "消息类型:" << message.type;
        qDebug() << "数据字段:";
        for (auto it = message.data.begin(); it != message.data.end(); ++it) {
            qDebug() << "  " << it.key() << "=" << it.value().toString();
        }
        qDebug() << "===============";
    }

    void onMessageReceived(const NetworkMessage &message, QTcpSocket *client) {
        qDebug() << "收到消息类型:" << message.type;

        switch (message.type) {
        case CONNECTION_TEST_RESPONSE: {
            QString content = message.data["content"].toString();
            qDebug() << "测试消息:" << content;

            NetworkMessage reply;
            reply.type = CONNECTION_TEST_RESPONSE;
            reply.data["content"] = "服务器回复: " + content;
            networkManager.sendMessage(reply, client);
            break;
        }
        case LOGIN_REQUEST: {
            debugNetworkMessage(message, "LOGIN_REQUEST");

            QString username = message.data["username"].toString();
            QString password = message.data["password"].toString();

            DatabaseManager& db = DatabaseManager::getInstance();
            NetworkMessage reply;
            reply.type = LOGIN_RESPONSE;

            try {
                if (db.validateUser(username, password)) {
                    int userId = db.getUserId(username);

                    bool updateSuccess = db.updateLastLogin(username);
                    qDebug() << "更新最后登录时间结果:" << (updateSuccess ? "成功" : "失败");

                    reply.data["success"] = true;
                    reply.data["message"] = "登录成功";
                    reply.data["user_id"] = userId;
                    reply.data["username"] = username;

                    // 记录登录状态
                    loggedInUsers[username] = client;
                    socketToUser[client] = username;
                    qDebug() << "✅ 用户" << username << "登录成功，记录登录状态";

                } else {
                    reply.data["success"] = false;
                    reply.data["message"] = "用户名或密码错误";
                    qDebug() << "用户登录失败:" << username;
                }
            } catch (const std::exception &e) {
                reply.data["success"] = false;
                reply.data["message"] = QString("登录过程中发生异常: %1").arg(e.what());
                qDebug() << "❌ 登录异常:" << e.what();
            } catch (...) {
                reply.data["success"] = false;
                reply.data["message"] = "登录过程中发生未知异常";
                qDebug() << "❌ 登录未知异常";
            }

            networkManager.sendMessage(reply, client);
            break;
        }
        case REGISTER_REQUEST: {
            QString username = message.data["username"].toString();
            QString password = message.data["password"].toString();
            QString email = message.data["email"].toString();

            DatabaseManager& db = DatabaseManager::getInstance();
            NetworkMessage reply;
            reply.type = REGISTER_RESPONSE;

            try {
                if (db.createUser(username, password, email)) {
                    reply.data["success"] = true;
                    reply.data["message"] = "注册成功";
                    reply.data["username"] = username;
                    qDebug() << "用户注册成功:" << username;
                } else {
                    reply.data["success"] = false;
                    reply.data["message"] = "注册失败，用户名可能已存在";
                    qDebug() << "用户注册失败:" << username;
                }
            } catch (const std::exception &e) {
                reply.data["success"] = false;
                reply.data["message"] = QString("注册过程中发生异常: %1").arg(e.what());
                qDebug() << "❌ 注册异常:" << e.what();
            } catch (...) {
                reply.data["success"] = false;
                reply.data["message"] = "注册过程中发生未知异常";
                qDebug() << "❌ 注册未知异常";
            }

            networkManager.sendMessage(reply, client);
            break;
        }
        case CHECK_USERNAME_REQUEST: {
            QString username = message.data["username"].toString();

            DatabaseManager& db = DatabaseManager::getInstance();
            NetworkMessage reply;
            reply.type = CHECK_USERNAME_RESPONSE;

            try {
                bool exists = db.userExists(username);
                reply.data["exists"] = exists;
                reply.data["username"] = username;

                qDebug() << "检查用户名:" << username << (exists ? "已存在" : "可用");
            } catch (const std::exception &e) {
                reply.data["exists"] = false;
                reply.data["username"] = username;
                reply.data["error"] = QString("检查用户名时发生异常: %1").arg(e.what());
                qDebug() << "❌ 检查用户名异常:" << e.what();
            } catch (...) {
                reply.data["exists"] = false;
                reply.data["username"] = username;
                reply.data["error"] = "检查用户名时发生未知异常";
                qDebug() << "❌ 检查用户名未知异常";
            }

            networkManager.sendMessage(reply, client);
            break;
        }
        case FLIGHT_SEARCH_REQUEST: {
            QString departureCity = message.data["departure_city"].toString();
            QString arrivalCity = message.data["arrival_city"].toString();
            QString date = message.data["date"].toString();
            QString sortBy = message.data["sort_by"].toString("departure_time");
            bool sortAsc = message.data["sort_asc"].toBool(true);
            QString airline = message.data["airline"].toString();

            NetworkMessage reply;
            reply.type = FLIGHT_SEARCH_RESPONSE;

            try {
                QString connectionName = "flight_search_conn_" + QString::number(quintptr(client));
                if (QSqlDatabase::contains(connectionName)) {
                    QSqlDatabase::removeDatabase(connectionName);
                }

                QSqlDatabase db = QSqlDatabase::addDatabase("QODBC", connectionName);
                db.setDatabaseName("flight_booking_system");
                db.setUserName("root");
                db.setPassword("123Xyf");

                // 设置连接选项
                db.setConnectOptions("SQL_ATTR_CONNECTION_POOLING=SQL_CP_ONE_PER_HENV;"
                                     "SQL_ATTR_ODBC_VERSION=SQL_OV_ODBC3;");

                if (!db.open()) {
                    qDebug() << "航班查询数据库连接失败:" << db.lastError().text();
                    reply.data["success"] = false;
                    reply.data["message"] = "数据库连接失败";
                    networkManager.sendMessage(reply, client);
                    return;
                }

                QSqlQuery query(db);
                QString sql = "SELECT * FROM flights WHERE 1=1";

                if (!departureCity.isEmpty()) {
                    sql += " AND departure_city = ?";
                }
                if (!arrivalCity.isEmpty()) {
                    sql += " AND arrival_city = ?";
                }
                if (!date.isEmpty()) {
                    sql += " AND DATE(departure_time) = ?";
                }
                if (!airline.isEmpty()) {
                    sql += " AND airline = ?";
                    qDebug() << "航空公司筛选条件:" << airline;
                }

                if (sortBy == "price") {
                    sql += " ORDER BY price " + QString(sortAsc ? "ASC" : "DESC");
                } else if (sortBy == "duration") {
                    sql += " ORDER BY duration_minutes " + QString(sortAsc ? "ASC" : "DESC");
                } else {
                    sql += " ORDER BY departure_time " + QString(sortAsc ? "ASC" : "DESC");
                }

                query.prepare(sql);

                int paramIndex = 0;
                if (!departureCity.isEmpty()) {
                    query.bindValue(paramIndex++, departureCity);
                }
                if (!arrivalCity.isEmpty()) {
                    query.bindValue(paramIndex++, arrivalCity);
                }
                if (!date.isEmpty()) {
                    query.bindValue(paramIndex++, date);
                }
                if (!airline.isEmpty()) {
                    query.bindValue(paramIndex++, airline);
                }

                if (query.exec()) {
                    QJsonArray flightsArray;
                    int count = 0;
                    while (query.next()) {
                        count++;
                        Flight flight(
                            query.value("id").toInt(),
                            query.value("flight_number").toString(),
                            query.value("airline").toString(),
                            query.value("departure_city").toString(),
                            query.value("arrival_city").toString(),
                            query.value("departure_time").toDateTime(),
                            query.value("arrival_time").toDateTime(),
                            query.value("duration_minutes").toInt(),
                            query.value("price").toDouble(),
                            query.value("total_seats").toInt(),
                            query.value("available_seats").toInt(),
                            query.value("aircraft_type").toString(),
                            query.value("status").toString()
                            );
                        flightsArray.append(flight.toJson());
                    }
                    reply.data["success"] = true;
                    reply.data["flights"] = flightsArray;
                    reply.data["count"] = count;
                } else {
                    qDebug() << "SQL执行失败:" << query.lastError().text();
                    reply.data["success"] = false;
                    reply.data["message"] = "查询失败: " + query.lastError().text();
                }

                query.finish();
                db.close();
                QThread::msleep(10);
                QSqlDatabase::removeDatabase(connectionName);

            } catch (const std::exception &e) {
                reply.data["success"] = false;
                reply.data["message"] = QString("航班查询异常: %1").arg(e.what());
                qDebug() << "❌ 航班查询异常:" << e.what();
            } catch (...) {
                reply.data["success"] = false;
                reply.data["message"] = "航班查询过程中发生未知异常";
                qDebug() << "❌ 航班查询未知异常";
            }

            networkManager.sendMessage(reply, client);
            break;
        }
        case CABIN_SEARCH_REQUEST: {
            int flightId = message.data["flight_id"].toInt();

            NetworkMessage reply;
            reply.type = CABIN_SEARCH_RESPONSE;

            try {
                QString connectionName = "cabin_search_conn_" + QString::number(quintptr(client));
                if (QSqlDatabase::contains(connectionName)) {
                    QSqlDatabase::removeDatabase(connectionName);
                }

                QSqlDatabase db = QSqlDatabase::addDatabase("QODBC", connectionName);
                db.setDatabaseName("flight_booking_system");
                db.setUserName("root");
                db.setPassword("123Xyf");

                // 设置连接选项
                db.setConnectOptions("SQL_ATTR_CONNECTION_POOLING=SQL_CP_ONE_PER_HENV;"
                                     "SQL_ATTR_ODBC_VERSION=SQL_OV_ODBC3;");

                if (!db.open()) {
                    qDebug() << "舱位查询数据库连接失败:" << db.lastError().text();
                    reply.data["success"] = false;
                    reply.data["message"] = "数据库连接失败";
                    networkManager.sendMessage(reply, client);
                    return;
                }

                QSqlQuery query(db);
                query.prepare(
                    "SELECT "
                    "CONVERT(id, CHAR) as id_str, "
                    "CONVERT(flight_id, CHAR) as flight_id_str, "
                    "cabin_type, "
                    "price, "
                    "CONVERT(available_seats, CHAR) as available_seats_str, "
                    "CONVERT(total_seats, CHAR) as total_seats_str, "
                    "baggage_allowance, "
                    "amenities "
                    "FROM cabins WHERE flight_id = ?"
                    );
                query.bindValue(0, flightId);

                if (query.exec()) {
                    QJsonArray cabinsArray;

                    while (query.next()) {
                        QString idStr = query.value("id_str").toString();
                        QString flightIdStr = query.value("flight_id_str").toString();
                        QString cabinType = query.value("cabin_type").toString();
                        double price = query.value("price").toDouble();
                        QString availSeatsStr = query.value("available_seats_str").toString();
                        QString totalSeatsStr = query.value("total_seats_str").toString();
                        QString baggageAllowance = query.value("baggage_allowance").toString();
                        QString amenities = query.value("amenities").toString();

                        int id = idStr.toInt();
                        int dbFlightId = flightIdStr.toInt();
                        int availableSeats = availSeatsStr.toInt();
                        int totalSeats = totalSeatsStr.toInt();

                        Cabin cabin(id, dbFlightId, cabinType, price, availableSeats, totalSeats,
                                    baggageAllowance, amenities);

                        cabinsArray.append(cabin.toJson());
                    }

                    reply.data["success"] = true;
                    reply.data["cabins"] = cabinsArray;

                } else {
                    qDebug() << "查询失败:" << query.lastError().text();
                    reply.data["success"] = false;
                    reply.data["message"] = "查询失败: " + query.lastError().text();
                }

                query.finish();
                db.close();
                QThread::msleep(10);
                QSqlDatabase::removeDatabase(connectionName);

            } catch (const std::exception &e) {
                reply.data["success"] = false;
                reply.data["message"] = QString("舱位查询异常: %1").arg(e.what());
                qDebug() << "❌ 舱位查询异常:" << e.what();
            } catch (...) {
                reply.data["success"] = false;
                reply.data["message"] = "舱位查询过程中发生未知异常";
                qDebug() << "❌ 舱位查询未知异常";
            }

            networkManager.sendMessage(reply, client);
            break;
        }
        case WALLET_QUERY_REQUEST: {
            debugNetworkMessage(message, "WALLET_QUERY_REQUEST");

            QString username = message.data["username"].toString();

            // 如果消息中没有用户名，尝试从socket映射中获取
            if (username.isEmpty() && socketToUser.contains(client)) {
                username = socketToUser[client];
                qDebug() << "⚠️ 消息中用户名为空，从socket映射中恢复:" << username;
            }

            if (username.isEmpty()) {
                qDebug() << "❌ 错误：用户名为空！返回错误信息";

                NetworkMessage reply;
                reply.type = WALLET_QUERY_RESPONSE;
                reply.data["success"] = false;
                reply.data["message"] = "用户未登录或用户名不能为空";
                reply.data["debug_info"] = "服务器收到的username字段为空";
                reply.data["request_type"] = "WALLET_QUERY_REQUEST";

                networkManager.sendMessage(reply, client);
                break;
            }

            qDebug() << "✅ 收到用户名:" << username;

            NetworkMessage reply;
            reply.type = WALLET_QUERY_RESPONSE;
            reply.data["username"] = username;  // 总是返回用户名

            try {
                DatabaseManager& db = DatabaseManager::getInstance();
                double balance = 0.0;

                if (db.queryUserBalance(username, balance)) {
                    reply.data["success"] = true;
                    reply.data["balance"] = balance;
                    reply.data["message"] = "查询成功";
                    qDebug() << "✅ 用户" << username << "余额查询成功:" << balance;
                } else {
                    reply.data["success"] = false;
                    reply.data["message"] = "查询余额失败，用户不存在或钱包未初始化";
                    qDebug() << "❌ 用户" << username << "余额查询失败";
                }
            } catch (const std::exception &e) {
                reply.data["success"] = false;
                reply.data["message"] = QString("余额查询异常: %1").arg(e.what());
                qDebug() << "❌ 余额查询异常:" << e.what();
            } catch (...) {
                reply.data["success"] = false;
                reply.data["message"] = "余额查询过程中发生未知异常";
                qDebug() << "❌ 余额查询未知异常";
            }

            networkManager.sendMessage(reply, client);
            break;
        }
        case RECHARGE_REQUEST: {
            debugNetworkMessage(message, "RECHARGE_REQUEST");

            QString username = message.data["username"].toString();
            double amount = message.data["amount"].toDouble();

            // 如果消息中没有用户名，尝试从socket映射中获取
            if (username.isEmpty() && socketToUser.contains(client)) {
                username = socketToUser[client];
                qDebug() << "⚠️ 消息中用户名为空，从socket映射中恢复:" << username;
            }

            if (username.isEmpty()) {
                qDebug() << "❌ 错误：充值用户名为空！";

                NetworkMessage reply;
                reply.type = RECHARGE_RESPONSE;
                reply.data["success"] = false;
                reply.data["message"] = "用户未登录或用户名不能为空";
                reply.data["debug_info"] = "服务器收到的username字段为空";

                networkManager.sendMessage(reply, client);
                break;
            }

            if (amount <= 0) {
                NetworkMessage reply;
                reply.type = RECHARGE_RESPONSE;
                reply.data["success"] = false;
                reply.data["message"] = "充值金额必须大于0";
                reply.data["username"] = username;
                networkManager.sendMessage(reply, client);
                break;
            }

            NetworkMessage reply;
            reply.type = RECHARGE_RESPONSE;
            reply.data["username"] = username;  // 总是返回用户名

            try {
                DatabaseManager& db = DatabaseManager::getInstance();
                double newBalance = 0.0;

                bool success = db.rechargeUserWallet(username, amount, newBalance);

                if (success) {
                    reply.data["success"] = true;
                    reply.data["message"] = "充值成功";
                    reply.data["new_balance"] = newBalance;
                    reply.data["recharged_amount"] = amount;
                    qDebug() << "✅ 用户" << username << "充值成功，新余额:" << newBalance;
                } else {
                    reply.data["success"] = false;
                    reply.data["message"] = "充值失败，请检查用户是否存在或数据库连接问题";
                    qDebug() << "❌ 用户" << username << "充值失败";
                }
            } catch (const std::exception &e) {
                reply.data["success"] = false;
                reply.data["message"] = QString("充值过程中发生异常: %1").arg(e.what());
                qDebug() << "❌ 充值异常:" << e.what();
            } catch (...) {
                reply.data["success"] = false;
                reply.data["message"] = "充值过程中发生未知异常";
                qDebug() << "❌ 充值未知异常";
            }

            networkManager.sendMessage(reply, client);
            break;
        }
        case RECHARGE_RECORDS_REQUEST: {
            debugNetworkMessage(message, "RECHARGE_RECORDS_REQUEST");

            QString username = message.data["username"].toString();

            // 如果消息中没有用户名，尝试从socket映射中获取
            if (username.isEmpty() && socketToUser.contains(client)) {
                username = socketToUser[client];
                qDebug() << "⚠️ 消息中用户名为空，从socket映射中恢复:" << username;
            }

            if (username.isEmpty()) {
                qDebug() << "❌ 错误：充值记录查询用户名为空！";

                NetworkMessage reply;
                reply.type = RECHARGE_RECORDS_RESPONSE;
                reply.data["success"] = false;
                reply.data["message"] = "用户未登录或用户名不能为空";

                networkManager.sendMessage(reply, client);
                break;
            }

            NetworkMessage reply;
            reply.type = RECHARGE_RECORDS_RESPONSE;
            reply.data["username"] = username;  // 总是返回用户名

            try {
                DatabaseManager& db = DatabaseManager::getInstance();

                // 使用新增的专用方法
                QJsonArray recordsArray = db.getRechargeRecords(username);

                if (!recordsArray.isEmpty()) {
                    reply.data["success"] = true;
                    reply.data["records"] = recordsArray;
                    reply.data["count"] = recordsArray.size();
                    qDebug() << "✅ 用户" << username << "充值记录查询成功，找到" << recordsArray.size() << "条记录";
                } else {
                    // 检查用户是否存在
                    int userId = db.getUserId(username);
                    if (userId <= 0) {
                        reply.data["success"] = false;
                        reply.data["message"] = "用户不存在";
                        qDebug() << "❌ 用户" << username << "不存在";
                    } else {
                        // 用户存在但没有充值记录
                        reply.data["success"] = true;
                        reply.data["records"] = QJsonArray();
                        reply.data["count"] = 0;
                        qDebug() << "✅ 用户" << username << "存在但没有充值记录";
                    }
                }
            } catch (const std::exception &e) {
                reply.data["success"] = false;
                reply.data["message"] = QString("充值记录查询异常: %1").arg(e.what());
                qDebug() << "❌ 充值记录查询异常:" << e.what();
            } catch (...) {
                reply.data["success"] = false;
                reply.data["message"] = "充值记录查询过程中发生未知异常";
                qDebug() << "❌ 充值记录查询未知异常";
            }

            networkManager.sendMessage(reply, client);
            break;
        }
        case BOOKING_REQUEST: {
            int flightId = message.data["flight_id"].toInt();
            int cabinId = message.data["cabin_id"].toInt();
            QString username = message.data["username"].toString();
            QString passengerName = message.data["passenger_name"].toString();
            QString passengerId = message.data["passenger_id"].toString();
            QString passengerPhone = message.data["passenger_phone"].toString();
            double totalPrice = message.data["total_price"].toDouble();

            qDebug() << "=== 预订请求（带余额检查）===";
            qDebug() << "用户名:" << username;
            qDebug() << "航班ID:" << flightId;
            qDebug() << "舱位ID:" << cabinId;
            qDebug() << "乘客:" << passengerName;
            qDebug() << "总价:" << totalPrice;

            NetworkMessage reply;
            reply.type = BOOKING_RESPONSE;
            reply.data["username"] = username;  // 总是返回用户名

            try {
                DatabaseManager& db = DatabaseManager::getInstance();
                QString bookingNumber;
                double newBalance = 0.0;

                if (db.processBooking(username, flightId, cabinId, passengerName, passengerId,
                                      passengerPhone, totalPrice, bookingNumber, newBalance)) {
                    reply.data["success"] = true;
                    reply.data["message"] = "预订成功";
                    reply.data["booking_number"] = bookingNumber;
                    reply.data["new_balance"] = newBalance;
                    qDebug() << "预订成功，订单号:" << bookingNumber << "，新余额:" << newBalance;
                } else {
                    reply.data["success"] = false;
                    reply.data["message"] = "预订失败，可能的原因：余额不足、座位已售罄或用户不存在";
                    qDebug() << "预订失败";
                }
            } catch (const std::exception &e) {
                reply.data["success"] = false;
                reply.data["message"] = QString("预订过程中发生异常: %1").arg(e.what());
                qDebug() << "❌ 预订异常:" << e.what();
            } catch (...) {
                reply.data["success"] = false;
                reply.data["message"] = "预订过程中发生未知异常";
                qDebug() << "❌ 预订未知异常";
            }

            networkManager.sendMessage(reply, client);
            break;
        }
        case ORDER_CANCEL_REQUEST: {
            int orderId = message.data["order_id"].toInt();
            QString username = message.data["username"].toString();

            qDebug() << "=== 取消订单请求 ===";
            qDebug() << "用户名:" << username;
            qDebug() << "订单ID:" << orderId;

            NetworkMessage reply;
            reply.type = ORDER_CANCEL_RESPONSE;
            reply.data["username"] = username;  // 总是返回用户名

            try {
                QString connectionName = "order_cancel_conn_" + QString::number(quintptr(client));
                if (QSqlDatabase::contains(connectionName)) {
                    QSqlDatabase::removeDatabase(connectionName);
                }

                QSqlDatabase db = QSqlDatabase::addDatabase("QODBC", connectionName);
                db.setDatabaseName("flight_booking_system");
                db.setUserName("root");
                db.setPassword("123Xyf");

                // 设置连接选项
                db.setConnectOptions("SQL_ATTR_CONNECTION_POOLING=SQL_CP_ONE_PER_HENV;"
                                     "SQL_ATTR_ODBC_VERSION=SQL_OV_ODBC3;");

                if (!db.open()) {
                    qDebug() << "订单取消数据库连接失败:" << db.lastError().text();
                    reply.data["success"] = false;
                    reply.data["message"] = "数据库连接失败";
                    networkManager.sendMessage(reply, client);
                    return;
                }

                if (!db.transaction()) {
                    qDebug() << "开始事务失败";
                    reply.data["success"] = false;
                    reply.data["message"] = "开始事务失败";
                    db.close();
                    QSqlDatabase::removeDatabase(connectionName);
                    networkManager.sendMessage(reply, client);
                    return;
                }

                // 1. 获取用户ID
                QSqlQuery userQuery(db);
                userQuery.prepare("SELECT id FROM users WHERE username = ?");
                userQuery.addBindValue(username);

                if (!userQuery.exec() || !userQuery.next()) {
                    db.rollback();
                    db.close();
                    QSqlDatabase::removeDatabase(connectionName);
                    reply.data["success"] = false;
                    reply.data["message"] = "用户不存在";
                    networkManager.sendMessage(reply, client);
                    return;
                }

                int userId = userQuery.value(0).toInt();

                // 2. 查询订单信息
                QSqlQuery orderQuery(db);
                orderQuery.prepare("SELECT flight_id, cabin_id, total_price, status FROM bookings "
                                   "WHERE id = ? AND user_id = ? AND status = '已预订'");
                orderQuery.addBindValue(orderId);
                orderQuery.addBindValue(userId);

                if (!orderQuery.exec() || !orderQuery.next()) {
                    db.rollback();
                    db.close();
                    QSqlDatabase::removeDatabase(connectionName);
                    reply.data["success"] = false;
                    reply.data["message"] = "订单不存在、状态不正确或已取消";
                    networkManager.sendMessage(reply, client);
                    return;
                }

                int flightId = orderQuery.value("flight_id").toInt();
                int cabinId = orderQuery.value("cabin_id").toInt();
                double refundAmount = orderQuery.value("total_price").toDouble();

                qDebug() << "订单信息 - 航班ID:" << flightId
                         << "，舱位ID:" << cabinId
                         << "，退款金额:" << refundAmount;

                // 3. 检查航班时间
                QSqlQuery flightQuery(db);
                flightQuery.prepare("SELECT departure_time FROM flights WHERE id = ?");
                flightQuery.addBindValue(flightId);

                if (!flightQuery.exec() || !flightQuery.next()) {
                    db.rollback();
                    db.close();
                    QSqlDatabase::removeDatabase(connectionName);
                    reply.data["success"] = false;
                    reply.data["message"] = "航班不存在";
                    networkManager.sendMessage(reply, client);
                    return;
                }

                QDateTime departureTime = flightQuery.value(0).toDateTime();
                QDateTime currentTime = QDateTime::currentDateTime();
                if (departureTime < currentTime.addSecs(2 * 3600)) {
                    db.rollback();
                    db.close();
                    QSqlDatabase::removeDatabase(connectionName);
                    reply.data["success"] = false;
                    reply.data["message"] = "航班即将起飞，无法取消订单";
                    networkManager.sendMessage(reply, client);
                    return;
                }

                // 4. 退款到钱包
                DatabaseManager& dbManager = DatabaseManager::getInstance();
                double newBalance = 0.0;

                if (!dbManager.rechargeUserWallet(username, refundAmount, newBalance)) {
                    db.rollback();
                    db.close();
                    QSqlDatabase::removeDatabase(connectionName);
                    reply.data["success"] = false;
                    reply.data["message"] = "退款到钱包失败";
                    networkManager.sendMessage(reply, client);
                    return;
                }

                // 5. 恢复座位
                QSqlQuery updateCabinQuery(db);
                updateCabinQuery.prepare("UPDATE cabins SET available_seats = available_seats + 1 WHERE id = ?");
                updateCabinQuery.addBindValue(cabinId);

                if (!updateCabinQuery.exec()) {
                    qDebug() << "恢复舱位座位失败:" << updateCabinQuery.lastError().text();
                    db.rollback();
                    db.close();
                    QSqlDatabase::removeDatabase(connectionName);
                    reply.data["success"] = false;
                    reply.data["message"] = "恢复舱位座位失败";
                    networkManager.sendMessage(reply, client);
                    return;
                }

                QSqlQuery updateFlightQuery(db);
                updateFlightQuery.prepare("UPDATE flights SET available_seats = available_seats + 1 WHERE id = ?");
                updateFlightQuery.addBindValue(flightId);

                if (!updateFlightQuery.exec()) {
                    qDebug() << "恢复航班座位失败:" << updateFlightQuery.lastError().text();
                    db.rollback();
                    db.close();
                    QSqlDatabase::removeDatabase(connectionName);
                    reply.data["success"] = false;
                    reply.data["message"] = "恢复航班座位失败";
                    networkManager.sendMessage(reply, client);
                    return;
                }

                // 6. 更新订单状态
                QSqlQuery updateOrderQuery(db);
                updateOrderQuery.prepare("UPDATE bookings SET status = '已取消', cancel_time = NOW() WHERE id = ?");
                updateOrderQuery.addBindValue(orderId);

                if (!updateOrderQuery.exec()) {
                    qDebug() << "更新订单状态失败:" << updateOrderQuery.lastError().text();
                    db.rollback();
                    db.close();
                    QSqlDatabase::removeDatabase(connectionName);
                    reply.data["success"] = false;
                    reply.data["message"] = "更新订单状态失败";
                    networkManager.sendMessage(reply, client);
                    return;
                }

                // 7. 提交事务
                if (!db.commit()) {
                    qDebug() << "提交事务失败:" << db.lastError().text();
                    db.rollback();
                    reply.data["success"] = false;
                    reply.data["message"] = "提交事务失败";
                } else {
                    QString bookingNumber = QString("BK%1").arg(orderId, 8, 10, QChar('0'));

                    reply.data["success"] = true;
                    reply.data["message"] = "订单取消成功";
                    reply.data["refund_amount"] = refundAmount;
                    reply.data["new_balance"] = newBalance;
                    reply.data["booking_number"] = bookingNumber;

                    qDebug() << "订单取消成功，用户:" << username
                             << "，订单ID:" << orderId
                             << "，退款:" << refundAmount
                             << "，新余额:" << newBalance;
                }

                userQuery.finish();
                orderQuery.finish();
                flightQuery.finish();
                updateCabinQuery.finish();
                updateFlightQuery.finish();
                updateOrderQuery.finish();

                db.close();
                QThread::msleep(10);
                QSqlDatabase::removeDatabase(connectionName);

            } catch (const std::exception &e) {
                reply.data["success"] = false;
                reply.data["message"] = QString("订单取消异常: %1").arg(e.what());
                qDebug() << "❌ 订单取消异常:" << e.what();
            } catch (...) {
                reply.data["success"] = false;
                reply.data["message"] = "订单取消过程中发生未知异常";
                qDebug() << "❌ 订单取消未知异常";
            }

            networkManager.sendMessage(reply, client);
            break;
        }
        case ORDER_LIST_REQUEST: {
            debugNetworkMessage(message, "ORDER_LIST_REQUEST");

            QString username = message.data["username"].toString();

            // 如果消息中没有用户名，尝试从socket映射中获取
            if (username.isEmpty() && socketToUser.contains(client)) {
                username = socketToUser[client];
                qDebug() << "⚠️ 消息中用户名为空，从socket映射中恢复:" << username;
            }

            if (username.isEmpty()) {
                qDebug() << "❌ 错误：订单查询用户名为空！";

                NetworkMessage reply;
                reply.type = ORDER_LIST_RESPONSE;
                reply.data["success"] = false;
                reply.data["message"] = "用户未登录或用户名不能为空";

                networkManager.sendMessage(reply, client);
                break;
            }

            NetworkMessage reply;
            reply.type = ORDER_LIST_RESPONSE;
            reply.data["username"] = username;  // 总是返回用户名

            try {
                QString connectionName = "order_list_conn_" + QString::number(quintptr(client));
                if (QSqlDatabase::contains(connectionName)) {
                    QSqlDatabase::removeDatabase(connectionName);
                }

                QSqlDatabase db = QSqlDatabase::addDatabase("QODBC", connectionName);
                db.setDatabaseName("flight_booking_system");
                db.setUserName("root");
                db.setPassword("123Xyf");

                // 设置连接选项
                db.setConnectOptions("SQL_ATTR_CONNECTION_POOLING=SQL_CP_ONE_PER_HENV;"
                                     "SQL_ATTR_ODBC_VERSION=SQL_OV_ODBC3;");

                if (!db.open()) {
                    qDebug() << "订单查询数据库连接失败:" << db.lastError().text();
                    reply.data["success"] = false;
                    reply.data["message"] = "数据库连接失败";
                    networkManager.sendMessage(reply, client);
                    return;
                }

                QString sql = QString(
                    "SELECT "
                    "b.id, "
                    "CONCAT('BK', "
                    "YEAR(b.booking_time), "
                    "LPAD(MONTH(b.booking_time), 2, '0'), "
                    "LPAD(DAY(b.booking_time), 2, '0'), "
                    "LPAD(b.id, 4, '0')) AS booking_number, "
                    "f.departure_city, "
                    "f.arrival_city, "
                    "DATE(f.departure_time) AS travel_date, "
                    "b.total_price, "
                    "CASE b.status "
                    "    WHEN '已预订' THEN 1 "
                    "    WHEN '已取消' THEN 2 "
                    "    ELSE 0 "
                    "END AS status "
                    "FROM bookings b "
                    "JOIN flights f ON b.flight_id = f.id "
                    "WHERE b.user_id = (SELECT id FROM users WHERE username = ?) "
                    "ORDER BY b.booking_time DESC"
                    );

                qDebug() << "执行SQL: " << sql;

                QSqlQuery query(db);
                query.prepare(sql);
                query.addBindValue(username);

                if (query.exec()) {
                    QJsonArray ordersArray;
                    int count = 0;

                    while (query.next()) {
                        QJsonObject orderObj;

                        orderObj["order_id"] = query.value(0).toInt();
                        orderObj["booking_number"] = query.value(1).toString();

                        QString departure = query.value(2).toString();
                        QString arrival = query.value(3).toString();
                        orderObj["flight_info"] = departure + " → " + arrival;

                        QDate travelDate = query.value(4).toDate();
                        orderObj["date"] = travelDate.toString("yyyy-MM-dd");

                        orderObj["price"] = query.value(5).toDouble();
                        orderObj["status"] = query.value(6).toInt();

                        ordersArray.append(orderObj);
                        count++;

                        qDebug() << "订单" << count << ":"
                                 << orderObj["booking_number"].toString()
                                 << orderObj["flight_info"].toString();
                    }

                    reply.data["success"] = true;
                    reply.data["orders"] = ordersArray;
                    reply.data["count"] = count;
                    qDebug() << "✅ 用户" << username << "订单查询成功，找到" << count << "个订单";

                } else {
                    QString errorText = query.lastError().text();
                    qDebug() << "❌ 用户" << username << "订单查询失败:" << errorText;

                    reply.data["success"] = false;
                    reply.data["message"] = "查询订单失败: " + errorText;
                }

                query.finish();
                db.close();
                QThread::msleep(10);
                QSqlDatabase::removeDatabase(connectionName);

            } catch (const std::exception &e) {
                reply.data["success"] = false;
                reply.data["message"] = QString("订单查询异常: %1").arg(e.what());
                qDebug() << "❌ 订单查询异常:" << e.what();
            } catch (...) {
                reply.data["success"] = false;
                reply.data["message"] = "订单查询过程中发生未知异常";
                qDebug() << "❌ 订单查询未知异常";
            }

            networkManager.sendMessage(reply, client);
            break;
        }
        case CHANGE_USERNAME_REQUEST: {
            QString oldUsername = message.data["old_username"].toString();
            QString newUsername = message.data["new_username"].toString();

            qDebug() << "=== 修改用户名请求 ===";
            qDebug() << "原用户名:" << oldUsername;
            qDebug() << "新用户名:" << newUsername;

            NetworkMessage reply;
            reply.type = CHANGE_USERNAME_RESPONSE;
            reply.data["old_username"] = oldUsername;
            reply.data["new_username"] = newUsername;

            try {
                QString connectionName = "change_username_conn_" + QString::number(quintptr(client));
                if (QSqlDatabase::contains(connectionName)) {
                    QSqlDatabase::removeDatabase(connectionName);
                }

                QSqlDatabase db = QSqlDatabase::addDatabase("QODBC", connectionName);
                db.setDatabaseName("flight_booking_system");
                db.setUserName("root");
                db.setPassword("123Xyf");

                // 设置连接选项
                db.setConnectOptions("SQL_ATTR_CONNECTION_POOLING=SQL_CP_ONE_PER_HENV;"
                                     "SQL_ATTR_ODBC_VERSION=SQL_OV_ODBC3;");

                if (!db.open()) {
                    qDebug() << "修改用户名数据库连接失败:" << db.lastError().text();
                    reply.data["success"] = false;
                    reply.data["message"] = "数据库连接失败";
                    networkManager.sendMessage(reply, client);
                    return;
                }

                if (!db.transaction()) {
                    qDebug() << "开始事务失败";
                    reply.data["success"] = false;
                    reply.data["message"] = "开始事务失败";
                    db.close();
                    QSqlDatabase::removeDatabase(connectionName);
                    networkManager.sendMessage(reply, client);
                    return;
                }

                // 1. 检查原用户是否存在
                QSqlQuery checkOldUser(db);
                checkOldUser.prepare("SELECT id FROM users WHERE username = ?");
                checkOldUser.addBindValue(oldUsername);

                if (!checkOldUser.exec() || !checkOldUser.next()) {
                    db.rollback();
                    db.close();
                    QSqlDatabase::removeDatabase(connectionName);
                    reply.data["success"] = false;
                    reply.data["message"] = "原用户名不存在";
                    networkManager.sendMessage(reply, client);
                    return;
                }

                int userId = checkOldUser.value(0).toInt();

                // 2. 检查新用户名是否已存在
                QSqlQuery checkNewUser(db);
                checkNewUser.prepare("SELECT id FROM users WHERE username = ?");
                checkNewUser.addBindValue(newUsername);

                if (checkNewUser.exec() && checkNewUser.next()) {
                    db.rollback();
                    db.close();
                    QSqlDatabase::removeDatabase(connectionName);
                    reply.data["success"] = false;
                    reply.data["message"] = "新用户名已存在，请选择其他用户名";
                    networkManager.sendMessage(reply, client);
                    return;
                }

                // 3. 更新用户名
                QSqlQuery updateQuery(db);
                updateQuery.prepare("UPDATE users SET username = ? WHERE id = ?");
                updateQuery.addBindValue(newUsername);
                updateQuery.addBindValue(userId);

                if (!updateQuery.exec()) {
                    db.rollback();
                    db.close();
                    QSqlDatabase::removeDatabase(connectionName);
                    reply.data["success"] = false;
                    reply.data["message"] = "更新用户名失败: " + updateQuery.lastError().text();
                    networkManager.sendMessage(reply, client);
                    return;
                }

                // 4. 提交事务
                if (!db.commit()) {
                    qDebug() << "提交事务失败:" << db.lastError().text();
                    db.rollback();
                    reply.data["success"] = false;
                    reply.data["message"] = "提交事务失败";
                } else {
                    reply.data["success"] = true;
                    reply.data["message"] = "用户名修改成功";

                    // 更新登录状态映射
                    if (loggedInUsers.contains(oldUsername) && loggedInUsers[oldUsername] == client) {
                        loggedInUsers.remove(oldUsername);
                        loggedInUsers[newUsername] = client;
                        socketToUser[client] = newUsername;
                    }

                    qDebug() << "用户名修改成功:" << oldUsername << "->" << newUsername;
                }

                checkOldUser.finish();
                checkNewUser.finish();
                updateQuery.finish();

                db.close();
                QThread::msleep(10);
                QSqlDatabase::removeDatabase(connectionName);

            } catch (const std::exception &e) {
                reply.data["success"] = false;
                reply.data["message"] = QString("修改用户名异常: %1").arg(e.what());
                qDebug() << "❌ 修改用户名异常:" << e.what();
            } catch (...) {
                reply.data["success"] = false;
                reply.data["message"] = "修改用户名过程中发生未知异常";
                qDebug() << "❌ 修改用户名未知异常";
            }

            networkManager.sendMessage(reply, client);
            break;
        }
        case CHANGE_PASSWORD_REQUEST: {
            QString username = message.data["username"].toString();
            QString oldPassword = message.data["old_password"].toString();
            QString newPassword = message.data["new_password"].toString();

            qDebug() << "=== 修改密码请求 ===";
            qDebug() << "用户名:" << username;

            NetworkMessage reply;
            reply.type = CHANGE_PASSWORD_RESPONSE;
            reply.data["username"] = username;  // 总是返回用户名

            try {
                QString connectionName = "change_password_conn_" + QString::number(quintptr(client));
                if (QSqlDatabase::contains(connectionName)) {
                    QSqlDatabase::removeDatabase(connectionName);
                }

                QSqlDatabase db = QSqlDatabase::addDatabase("QODBC", connectionName);
                db.setDatabaseName("flight_booking_system");
                db.setUserName("root");
                db.setPassword("123Xyf");

                // 设置连接选项
                db.setConnectOptions("SQL_ATTR_CONNECTION_POOLING=SQL_CP_ONE_PER_HENV;"
                                     "SQL_ATTR_ODBC_VERSION=SQL_OV_ODBC3;");

                if (!db.open()) {
                    qDebug() << "修改密码数据库连接失败:" << db.lastError().text();
                    reply.data["success"] = false;
                    reply.data["message"] = "数据库连接失败";
                    networkManager.sendMessage(reply, client);
                    return;
                }

                if (!db.transaction()) {
                    qDebug() << "开始事务失败";
                    reply.data["success"] = false;
                    reply.data["message"] = "开始事务失败";
                    db.close();
                    QSqlDatabase::removeDatabase(connectionName);
                    networkManager.sendMessage(reply, client);
                    return;
                }

                // 1. 检查用户和原密码
                QSqlQuery checkUser(db);
                checkUser.prepare("SELECT id, password FROM users WHERE username = ?");
                checkUser.addBindValue(username);

                if (!checkUser.exec() || !checkUser.next()) {
                    db.rollback();
                    db.close();
                    QSqlDatabase::removeDatabase(connectionName);
                    reply.data["success"] = false;
                    reply.data["message"] = "用户不存在";
                    networkManager.sendMessage(reply, client);
                    return;
                }

                int userId = checkUser.value("id").toInt();
                QString storedPassword = checkUser.value("password").toString();

                if (storedPassword != oldPassword) {
                    db.rollback();
                    db.close();
                    QSqlDatabase::removeDatabase(connectionName);
                    reply.data["success"] = false;
                    reply.data["message"] = "原密码错误";
                    networkManager.sendMessage(reply, client);
                    return;
                }

                // 2. 检查新密码是否与原密码相同
                if (oldPassword == newPassword) {
                    db.rollback();
                    db.close();
                    QSqlDatabase::removeDatabase(connectionName);
                    reply.data["success"] = false;
                    reply.data["message"] = "新密码不能与原密码相同";
                    networkManager.sendMessage(reply, client);
                    return;
                }

                // 3. 检查新密码长度
                if (newPassword.length() < 6 || newPassword.length() > 20) {
                    db.rollback();
                    db.close();
                    QSqlDatabase::removeDatabase(connectionName);
                    reply.data["success"] = false;
                    reply.data["message"] = "密码长度应在6-20个字符之间";
                    networkManager.sendMessage(reply, client);
                    return;
                }

                // 4. 更新密码
                QSqlQuery updateQuery(db);
                updateQuery.prepare("UPDATE users SET password = ? WHERE id = ?");
                updateQuery.addBindValue(newPassword);
                updateQuery.addBindValue(userId);

                if (!updateQuery.exec()) {
                    db.rollback();
                    db.close();
                    QSqlDatabase::removeDatabase(connectionName);
                    reply.data["success"] = false;
                    reply.data["message"] = "更新密码失败: " + updateQuery.lastError().text();
                    networkManager.sendMessage(reply, client);
                    return;
                }

                // 5. 提交事务
                if (!db.commit()) {
                    qDebug() << "提交事务失败:" << db.lastError().text();
                    db.rollback();
                    reply.data["success"] = false;
                    reply.data["message"] = "提交事务失败";
                } else {
                    reply.data["success"] = true;
                    reply.data["message"] = "密码修改成功";
                    qDebug() << "密码修改成功，用户:" << username;
                }

                checkUser.finish();
                updateQuery.finish();

                db.close();
                QThread::msleep(10);
                QSqlDatabase::removeDatabase(connectionName);

            } catch (const std::exception &e) {
                reply.data["success"] = false;
                reply.data["message"] = QString("修改密码异常: %1").arg(e.what());
                qDebug() << "❌ 修改密码异常:" << e.what();
            } catch (...) {
                reply.data["success"] = false;
                reply.data["message"] = "修改密码过程中发生未知异常";
                qDebug() << "❌ 修改密码未知异常";
            }

            networkManager.sendMessage(reply, client);
            break;
        }
        default:
            qDebug() << "未知消息类型:" << message.type;

            // 使用已有的消息类型回复
            NetworkMessage reply;
            reply.type = CONNECTION_TEST_RESPONSE;  // 使用连接测试响应类型
            reply.data["success"] = false;
            reply.data["message"] = "未知的消息类型";
            reply.data["received_type"] = message.type;  // 包含接收到的类型
            networkManager.sendMessage(reply, client);
            break;
        }
    }

private:
    ServerNetworkManager networkManager;
    QMap<QString, QTcpSocket*> loggedInUsers;  // 用户名 -> socket映射
    QMap<QTcpSocket*, QString> socketToUser;   // socket -> 用户名映射
};

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    TestServer server;
    server.start();

    return a.exec();
}

#include "main.moc"
