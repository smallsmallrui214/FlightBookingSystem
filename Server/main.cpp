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
        welcomeMsg.type = TEST_MESSAGE;
        welcomeMsg.data["content"] = "欢迎连接到航班服务器!";
        networkManager.sendMessage(welcomeMsg, client);
    }

    void onMessageReceived(const NetworkMessage &message, QTcpSocket *client) {
        qDebug() << "收到消息类型:" << message.type;

        switch (message.type) {
        case TEST_MESSAGE: {
            QString content = message.data["content"].toString();
            qDebug() << "测试消息:" << content;

            NetworkMessage reply;
            reply.type = TEST_MESSAGE;
            reply.data["content"] = "服务器回复: " + content;
            networkManager.sendMessage(reply, client);
            break;
        }
        case LOGIN_REQUEST: {
            QString username = message.data["username"].toString();
            QString password = message.data["password"].toString();

            DatabaseManager& db = DatabaseManager::getInstance();
            NetworkMessage reply;
            reply.type = LOGIN_RESPONSE;

            if (db.validateUser(username, password)) {
                int userId = db.getUserId(username);

                bool updateSuccess = db.updateLastLogin(username);
                qDebug() << "更新最后登录时间结果:" << (updateSuccess ? "成功" : "失败");

                QSqlQuery timeQuery;
                timeQuery.prepare("SELECT last_login FROM users WHERE username = ?");
                timeQuery.addBindValue(username);
                if (timeQuery.exec() && timeQuery.next()) {
                    QDateTime lastLogin = timeQuery.value(0).toDateTime();
                    qDebug() << "用户" << username << "的最后登录时间:" << lastLogin.toString("yyyy-MM-dd hh:mm:ss");
                    reply.data["last_login"] = lastLogin.toString("yyyy-MM-dd hh:mm:ss");
                }

                reply.data["success"] = true;
                reply.data["message"] = "登录成功";
                reply.data["user_id"] = userId;
                reply.data["username"] = username;
                qDebug() << "用户登录成功:" << username << "ID:" << userId;
            } else {
                reply.data["success"] = false;
                reply.data["message"] = "用户名或密码错误";
                qDebug() << "用户登录失败:" << username;
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
            networkManager.sendMessage(reply, client);
            break;
        }
        case CHECK_USERNAME_REQUEST: {
            QString username = message.data["username"].toString();

            DatabaseManager& db = DatabaseManager::getInstance();
            NetworkMessage reply;
            reply.type = CHECK_USERNAME_RESPONSE;

            bool exists = db.userExists(username);
            reply.data["exists"] = exists;
            reply.data["username"] = username;

            qDebug() << "检查用户名:" << username << (exists ? "已存在" : "可用");
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

            QSqlQuery query;
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

            NetworkMessage reply;
            reply.type = FLIGHT_SEARCH_RESPONSE;

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
            } else {
                qDebug() << "SQL执行失败:" << query.lastError().text();
                reply.data["success"] = false;
                reply.data["message"] = "查询失败: " + query.lastError().text();
            }

            networkManager.sendMessage(reply, client);
            break;
        }
        case CABIN_SEARCH_REQUEST: {
            int flightId = message.data["flight_id"].toInt();

            // 使用CONVERT或CAST将整数字段转换为字符串，避免ODBC问题
            QSqlQuery query;
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

            NetworkMessage reply;
            reply.type = CABIN_SEARCH_RESPONSE;

            if (query.exec()) {

                QJsonArray cabinsArray;

                while (query.next()) {
                    // 以字符串形式获取，然后转换
                    QString idStr = query.value("id_str").toString();
                    QString flightIdStr = query.value("flight_id_str").toString();
                    QString cabinType = query.value("cabin_type").toString();
                    double price = query.value("price").toDouble();
                    QString availSeatsStr = query.value("available_seats_str").toString();
                    QString totalSeatsStr = query.value("total_seats_str").toString();
                    QString baggageAllowance = query.value("baggage_allowance").toString();
                    QString amenities = query.value("amenities").toString();

                    // 转换为整数
                    int id = idStr.toInt();
                    int dbFlightId = flightIdStr.toInt();
                    int availableSeats = availSeatsStr.toInt();
                    int totalSeats = totalSeatsStr.toInt();

                    // 创建Cabin对象
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

            networkManager.sendMessage(reply, client);
            break;
        }

        // =============== 新增：钱包查询处理 ===============
        case WALLET_QUERY_REQUEST: {  // 400
            QString username = message.data["username"].toString();
            qDebug() << "处理钱包查询请求，用户名:" << username;

            NetworkMessage reply;
            reply.type = WALLET_QUERY_RESPONSE;  // 401

            // 查询数据库
            QSqlQuery query;
            query.prepare("SELECT w.balance FROM wallets w "
                          "JOIN users u ON w.user_id = u.id "
                          "WHERE u.username = ?");
            query.addBindValue(username);

            if (query.exec() && query.next()) {
                double balance = query.value(0).toDouble();
                reply.data["success"] = true;
                reply.data["balance"] = balance;
                reply.data["message"] = "查询成功";
                qDebug() << "用户" << username << "余额查询成功:" << balance;
            } else {
                reply.data["success"] = false;
                reply.data["message"] = "查询余额失败，用户不存在或钱包未初始化";
                qDebug() << "用户" << username << "余额查询失败:" << query.lastError().text();
            }

            networkManager.sendMessage(reply, client);
            break;
        }

        // =============== 新增：充值处理 ===============
        case RECHARGE_REQUEST: {  // 402
            QString username = message.data["username"].toString();
            double amount = message.data["amount"].toDouble();

            qDebug() << "处理充值请求，用户:" << username << "，金额:" << amount;

            if (amount <= 0) {
                NetworkMessage reply;
                reply.type = RECHARGE_RESPONSE;  // 403
                reply.data["success"] = false;
                reply.data["message"] = "充值金额必须大于0";
                networkManager.sendMessage(reply, client);
                break;
            }

            NetworkMessage reply;
            reply.type = RECHARGE_RESPONSE;

            // 开始事务
            QSqlDatabase::database().transaction();

            try {
                // 1. 查询用户ID
                QSqlQuery userQuery;
                userQuery.prepare("SELECT id FROM users WHERE username = ?");
                userQuery.addBindValue(username);

                if (!userQuery.exec() || !userQuery.next()) {
                    throw std::runtime_error("用户不存在");
                }

                int userId = userQuery.value(0).toInt();

                // 2. 查询当前余额（如果没有钱包记录，创建一条）
                QSqlQuery balanceQuery;
                balanceQuery.prepare("SELECT balance FROM wallets WHERE user_id = ?");
                balanceQuery.addBindValue(userId);

                double oldBalance = 0.0;
                if (!balanceQuery.exec() || !balanceQuery.next()) {
                    // 如果钱包不存在，创建钱包
                    QSqlQuery createWalletQuery;
                    createWalletQuery.prepare("INSERT INTO wallets (user_id, balance) VALUES (?, 0.00)");
                    createWalletQuery.addBindValue(userId);

                    if (!createWalletQuery.exec()) {
                        throw std::runtime_error("创建钱包失败");
                    }

                    oldBalance = 0.0;
                } else {
                    oldBalance = balanceQuery.value(0).toDouble();
                }

                double newBalance = oldBalance + amount;

                // 3. 更新余额
                QSqlQuery updateQuery;
                updateQuery.prepare("UPDATE wallets SET balance = ? WHERE user_id = ?");
                updateQuery.addBindValue(newBalance);
                updateQuery.addBindValue(userId);

                if (!updateQuery.exec()) {
                    throw std::runtime_error("更新余额失败");
                }

                // 4. 记录充值历史
                QSqlQuery recordQuery;
                recordQuery.prepare("INSERT INTO recharge_records (user_id, amount, old_balance, new_balance) "
                                    "VALUES (?, ?, ?, ?)");
                recordQuery.addBindValue(userId);
                recordQuery.addBindValue(amount);
                recordQuery.addBindValue(oldBalance);
                recordQuery.addBindValue(newBalance);

                if (!recordQuery.exec()) {
                    throw std::runtime_error("记录充值历史失败");
                }

                // 提交事务
                QSqlDatabase::database().commit();

                reply.data["success"] = true;
                reply.data["message"] = "充值成功";
                reply.data["new_balance"] = newBalance;
                reply.data["recharged_amount"] = amount;

                qDebug() << "用户" << username << "充值成功，金额:" << amount
                         << "，旧余额:" << oldBalance << "，新余额:" << newBalance;

            } catch (const std::exception &e) {
                // 回滚事务
                QSqlDatabase::database().rollback();

                reply.data["success"] = false;
                reply.data["message"] = QString("充值失败: %1").arg(e.what());
                qDebug() << "充值失败:" << e.what();
            }

            networkManager.sendMessage(reply, client);
            break;
        }

        // =============== 修改：预订请求处理（带余额检查） ===============
        case BOOKING_REQUEST: {  // 300
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
            reply.type = BOOKING_RESPONSE;  // 301

            // 开始事务
            QSqlDatabase::database().transaction();

            try {
                // 1. 查询用户ID
                QSqlQuery userQuery;
                userQuery.prepare("SELECT id FROM users WHERE username = ?");
                userQuery.addBindValue(username);

                if (!userQuery.exec() || !userQuery.next()) {
                    throw std::runtime_error("用户不存在");
                }

                int userId = userQuery.value(0).toInt();

                // 2. 检查余额是否足够
                QSqlQuery balanceQuery;
                balanceQuery.prepare("SELECT balance FROM wallets WHERE user_id = ?");
                balanceQuery.addBindValue(userId);

                if (!balanceQuery.exec() || !balanceQuery.next()) {
                    throw std::runtime_error("钱包不存在，请先充值");
                }

                double currentBalance = balanceQuery.value(0).toDouble();
                if (currentBalance < totalPrice) {
                    throw std::runtime_error(QString("余额不足，需要¥%1，当前余额¥%2")
                                                 .arg(totalPrice, 0, 'f', 2)
                                                 .arg(currentBalance, 0, 'f', 2).toStdString());
                }

                double newBalance = currentBalance - totalPrice;

                // 3. 检查舱位可用性
                QSqlQuery checkQuery;
                checkQuery.prepare("SELECT available_seats FROM cabins WHERE id = ?");
                checkQuery.addBindValue(cabinId);

                if (!checkQuery.exec() || !checkQuery.next()) {
                    throw std::runtime_error("舱位不存在");
                }

                int availableSeats = checkQuery.value(0).toInt();
                if (availableSeats <= 0) {
                    throw std::runtime_error("该舱位已无可用座位");
                }

                // 4. 扣款
                QSqlQuery deductQuery;
                deductQuery.prepare("UPDATE wallets SET balance = ? WHERE user_id = ?");
                deductQuery.addBindValue(newBalance);
                deductQuery.addBindValue(userId);

                if (!deductQuery.exec()) {
                    throw std::runtime_error("扣款失败");
                }

                // 5. 生成订单号
                QString dateStr = QDateTime::currentDateTime().toString("yyyyMMdd");
                int randomNum = QRandomGenerator::global()->bounded(10000);
                QString bookingNumber = "BK" + dateStr + QString("%1").arg(randomNum, 4, 10, QChar('0'));

                // 6. 查询航班信息
                QSqlQuery flightQuery;
                flightQuery.prepare("SELECT departure_city, arrival_city, departure_time, "
                                    "arrival_time, flight_number, airline "
                                    "FROM flights WHERE id = ?");
                flightQuery.addBindValue(flightId);

                if (!flightQuery.exec() || !flightQuery.next()) {
                    throw std::runtime_error("航班不存在");
                }

                QString departureCity = flightQuery.value("departure_city").toString();
                QString arrivalCity = flightQuery.value("arrival_city").toString();
                QDateTime departureTime = flightQuery.value("departure_time").toDateTime();
                QDateTime arrivalTime = flightQuery.value("arrival_time").toDateTime();
                QString flightNumber = flightQuery.value("flight_number").toString();
                QString airline = flightQuery.value("airline").toString();

                // 7. 创建预订记录
                QSqlQuery insertQuery;
                insertQuery.prepare(
                    "INSERT INTO bookings (user_id, flight_id, cabin_id, "
                    "passenger_name, passenger_id, passenger_phone, total_price, status, booking_time) "
                    "VALUES (?, ?, ?, ?, ?, ?, ?, '已预订', NOW())"
                    );


                insertQuery.addBindValue(userId);               // user_id
                insertQuery.addBindValue(flightId);             // flight_id
                insertQuery.addBindValue(cabinId);              // cabin_id
                insertQuery.addBindValue(passengerName);        // passenger_name
                insertQuery.addBindValue(passengerId);          // passenger_id
                insertQuery.addBindValue(passengerPhone);       // passenger_phone
                insertQuery.addBindValue(totalPrice);           // total_price

                qDebug() << "插入预订记录，参数:";
                qDebug() << "user_id:" << userId;
                qDebug() << "flight_id:" << flightId;
                qDebug() << "cabin_id:" << cabinId;
                qDebug() << "passenger_name:" << passengerName;
                qDebug() << "total_price:" << totalPrice;

                if (!insertQuery.exec()) {
                    QString error = insertQuery.lastError().text();
                    QString driverError = insertQuery.lastError().driverText();

                    qDebug() << "=== 插入预订失败详细信息 ===";
                    qDebug() << "错误信息:" << error;
                    qDebug() << "驱动程序错误:" << driverError;

                    // 调试：检查实际插入的SQL
                    QString executedQuery = insertQuery.executedQuery();
                    if (!executedQuery.isEmpty()) {
                        qDebug() << "执行的SQL:" << executedQuery;
                    }

                    // 测试是否能执行简单查询
                    QSqlQuery testQuery("SELECT COUNT(*) FROM bookings");
                    if (testQuery.exec() && testQuery.next()) {
                        qDebug() << "测试查询成功，bookings表记录数:" << testQuery.value(0).toInt();
                    } else {
                        qDebug() << "测试查询失败:" << testQuery.lastError().text();
                    }

                    throw std::runtime_error("创建订单失败: " + error.toStdString());
                }

                int bookingId = insertQuery.lastInsertId().toInt();
                qDebug() << "预订创建成功，ID:" << bookingId;


                // 8. 更新舱位可用座位数
                QSqlQuery updateCabinQuery;
                updateCabinQuery.prepare("UPDATE cabins SET available_seats = available_seats - 1 WHERE id = ?");
                updateCabinQuery.addBindValue(cabinId);

                if (!updateCabinQuery.exec()) {
                    qDebug() << "更新舱位座位数失败:" << updateCabinQuery.lastError().text();
                    throw std::runtime_error("更新舱位座位数失败");
                }

                // 9. 更新航班可用座位数
                QSqlQuery updateFlightQuery;
                updateFlightQuery.prepare("UPDATE flights SET available_seats = available_seats - 1 WHERE id = ?");
                updateFlightQuery.addBindValue(flightId);

                if (!updateFlightQuery.exec()) {
                    qDebug() << "更新航班座位数失败:" << updateFlightQuery.lastError().text();
                    throw std::runtime_error("更新航班座位数失败");
                }

                // 提交事务
                QSqlDatabase::database().commit();

                reply.data["success"] = true;
                reply.data["message"] = "预订成功";
                reply.data["booking_number"] = bookingNumber;
                reply.data["booking_id"] = bookingId;
                reply.data["new_balance"] = newBalance;  // 返回新余额

                qDebug() << "预订成功，用户:" << username
                         << "，订单号:" << bookingNumber
                         << "，扣款:" << totalPrice
                         << "，新余额:" << newBalance;

            } catch (const std::exception &e) {
                // 回滚事务
                QSqlDatabase::database().rollback();

                reply.data["success"] = false;
                reply.data["message"] = QString("预订失败: %1").arg(e.what());
                qDebug() << "预订失败:" << e.what();
            }

            networkManager.sendMessage(reply, client);
            break;
        }


        case ORDER_CANCEL_REQUEST: {  // 304
            int orderId = message.data["order_id"].toInt();
            QString username = message.data["username"].toString();

            qDebug() << "=== 取消订单请求 ===";
            qDebug() << "用户名:" << username;
            qDebug() << "订单ID:" << orderId;

            NetworkMessage reply;
            reply.type = ORDER_CANCEL_RESPONSE;  // 305

            // 开始事务
            QSqlDatabase::database().transaction();

            try {
                // 1. 查询用户ID
                QSqlQuery userQuery;
                userQuery.prepare("SELECT id FROM users WHERE username = ?");
                userQuery.addBindValue(username);

                if (!userQuery.exec() || !userQuery.next()) {
                    throw std::runtime_error("用户不存在");
                }

                int userId = userQuery.value(0).toInt();

                // 2. 查询订单信息
                QSqlQuery orderQuery;
                orderQuery.prepare("SELECT flight_id, cabin_id, total_price, status FROM bookings "
                                   "WHERE id = ? AND user_id = ? AND status = '已预订'");
                orderQuery.addBindValue(orderId);
                orderQuery.addBindValue(userId);

                if (!orderQuery.exec() || !orderQuery.next()) {
                    throw std::runtime_error("订单不存在、状态不正确或已取消");
                }

                int flightId = orderQuery.value("flight_id").toInt();
                int cabinId = orderQuery.value("cabin_id").toInt();
                double refundAmount = orderQuery.value("total_price").toDouble();

                qDebug() << "订单信息 - 航班ID:" << flightId
                         << "，舱位ID:" << cabinId
                         << "，退款金额:" << refundAmount;

                // 3. 查询航班起飞时间
                QSqlQuery flightQuery;
                flightQuery.prepare("SELECT departure_time FROM flights WHERE id = ?");
                flightQuery.addBindValue(flightId);

                if (!flightQuery.exec() || !flightQuery.next()) {
                    throw std::runtime_error("航班不存在");
                }

                QDateTime departureTime = flightQuery.value(0).toDateTime();
                QDateTime currentTime = QDateTime::currentDateTime();
                if (departureTime < currentTime.addSecs(2 * 3600)) {
                    throw std::runtime_error("航班即将起飞，无法取消订单");
                }

                // 4. 检查钱包是否存在
                QSqlQuery balanceQuery;
                balanceQuery.prepare("SELECT balance FROM wallets WHERE user_id = ?");
                balanceQuery.addBindValue(userId);

                if (!balanceQuery.exec() || !balanceQuery.next()) {
                    throw std::runtime_error("钱包不存在");
                }

                double oldBalance = balanceQuery.value(0).toDouble();
                double newBalance = oldBalance + refundAmount;

                // 5. 退款
                QSqlQuery refundQuery;
                refundQuery.prepare("UPDATE wallets SET balance = ? WHERE user_id = ?");
                refundQuery.addBindValue(newBalance);
                refundQuery.addBindValue(userId);

                if (!refundQuery.exec()) {
                    throw std::runtime_error("退款失败");
                }

                // 6. 恢复舱位座位
                QSqlQuery updateCabinQuery;
                updateCabinQuery.prepare("UPDATE cabins SET available_seats = available_seats + 1 WHERE id = ?");
                updateCabinQuery.addBindValue(cabinId);

                if (!updateCabinQuery.exec()) {
                    qDebug() << "恢复舱位座位失败:" << updateCabinQuery.lastError().text();
                    throw std::runtime_error("恢复舱位座位失败");
                }

                // 7. 恢复航班座位
                QSqlQuery updateFlightQuery;
                updateFlightQuery.prepare("UPDATE flights SET available_seats = available_seats + 1 WHERE id = ?");
                updateFlightQuery.addBindValue(flightId);

                if (!updateFlightQuery.exec()) {
                    qDebug() << "恢复航班座位失败:" << updateFlightQuery.lastError().text();
                    throw std::runtime_error("恢复航班座位失败");
                }

                // 8. 删除订单（替代更新状态）
                QSqlQuery deleteOrderQuery;
                deleteOrderQuery.prepare("DELETE FROM bookings WHERE id = ?");
                deleteOrderQuery.addBindValue(orderId);

                if (!deleteOrderQuery.exec()) {
                    qDebug() << "删除订单失败:" << deleteOrderQuery.lastError().text();
                    throw std::runtime_error("删除订单失败");
                }

                qDebug() << "删除订单成功，删除了" << deleteOrderQuery.numRowsAffected() << "行";

                // 提交事务
                QSqlDatabase::database().commit();

                // 生成订单号用于显示
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

            } catch (const std::exception &e) {
                // 回滚事务
                QSqlDatabase::database().rollback();

                reply.data["success"] = false;
                reply.data["message"] = QString("取消失败: %1").arg(e.what());
                qDebug() << "订单取消失败:" << e.what();
            }

            networkManager.sendMessage(reply, client);
            break;
        }

        case ORDER_LIST_REQUEST: {
            QString username = message.data["username"].toString();
            qDebug() << "查询用户订单，用户名:" << username;

            NetworkMessage reply;
            reply.type = ORDER_LIST_RESPONSE;

            // 1. 查询用户ID
            QSqlQuery userIdQuery;
            userIdQuery.prepare("SELECT id FROM users WHERE username = ?");
            userIdQuery.addBindValue(username);

            if (!userIdQuery.exec() || !userIdQuery.next()) {
                reply.data["success"] = false;
                reply.data["message"] = "用户不存在";
                networkManager.sendMessage(reply, client);
                break;
            }

            int userId = userIdQuery.value(0).toInt();
            qDebug() << "用户ID:" << userId;

            // 2. 根据您的实际表结构修改查询
            // 方案A：JOIN flights 表获取信息
            QString sql = QString(
                              "SELECT "
                              "b.id, "                                // 订单ID
                              "CONCAT('BK', "
                              "YEAR(b.booking_time), "                // 使用YEAR函数
                              "LPAD(MONTH(b.booking_time), 2, '0'), " // 使用MONTH函数
                              "LPAD(DAY(b.booking_time), 2, '0'), "   // 使用DAY函数
                              "LPAD(b.id, 4, '0')) AS booking_number, "
                              "f.departure_city, "                    // 出发城市
                              "f.arrival_city, "                      // 到达城市
                              "DATE(f.departure_time) AS travel_date, "  // 旅行日期
                              "b.total_price, "                       // 总价格
                              "CASE b.status "
                              "    WHEN '已预订' THEN 1 "
                              "    WHEN '已取消' THEN 2 "
                              "    ELSE 0 "
                              "END AS status "
                              "FROM bookings b "
                              "JOIN flights f ON b.flight_id = f.id "
                              "WHERE b.user_id = %1 "
                              "ORDER BY b.booking_time DESC"
                              ).arg(userId);

            qDebug() << "执行SQL:" << sql;

            QSqlQuery query;
            if (query.exec(sql)) {
                QJsonArray ordersArray;
                int count = 0;

                while (query.next()) {
                    QJsonObject orderObj;

                    orderObj["order_id"] = query.value(0).toInt();           // b.id
                    orderObj["booking_number"] = query.value(1).toString();  // 生成的订单号

                    QString departure = query.value(2).toString();           // departure_city
                    QString arrival = query.value(3).toString();             // arrival_city
                    orderObj["flight_info"] = departure + " → " + arrival;

                    QDate travelDate = query.value(4).toDate();              // travel_date
                    orderObj["date"] = travelDate.toString("yyyy-MM-dd");

                    orderObj["price"] = query.value(5).toDouble();           // total_price
                    orderObj["status"] = query.value(6).toInt();             // 转换后的状态

                    ordersArray.append(orderObj);
                    count++;

                    qDebug() << "订单" << count << ":"
                             << orderObj["booking_number"].toString()
                             << orderObj["flight_info"].toString();
                }

                reply.data["success"] = true;
                reply.data["orders"] = ordersArray;
                reply.data["count"] = count;
                qDebug() << "查询成功，找到" << count << "个订单";

            } else {
                QString errorText = query.lastError().text();
                qDebug() << "查询失败:" << errorText;

                reply.data["success"] = false;
                reply.data["message"] = "查询订单失败: " + errorText;
            }

            networkManager.sendMessage(reply, client);
            break;
        }
        // =============== 新增：修改用户名处理 ===============
        case CHANGE_USERNAME_REQUEST: {  // 104
            QString oldUsername = message.data["old_username"].toString();
            QString newUsername = message.data["new_username"].toString();

            qDebug() << "=== 修改用户名请求 ===";
            qDebug() << "原用户名:" << oldUsername;
            qDebug() << "新用户名:" << newUsername;

            NetworkMessage reply;
            reply.type = CHANGE_USERNAME_RESPONSE;  // 105

            // 开始事务
            QSqlDatabase::database().transaction();

            try {
                // 1. 检查原用户名是否存在
                QSqlQuery checkOldUser;
                checkOldUser.prepare("SELECT id FROM users WHERE username = ?");
                checkOldUser.addBindValue(oldUsername);

                if (!checkOldUser.exec() || !checkOldUser.next()) {
                    throw std::runtime_error("原用户名不存在");
                }

                int userId = checkOldUser.value(0).toInt();

                // 2. 检查新用户名是否已存在
                QSqlQuery checkNewUser;
                checkNewUser.prepare("SELECT id FROM users WHERE username = ?");
                checkNewUser.addBindValue(newUsername);

                if (checkNewUser.exec() && checkNewUser.next()) {
                    throw std::runtime_error("新用户名已存在，请选择其他用户名");
                }

                // 3. 更新用户名
                QSqlQuery updateQuery;
                updateQuery.prepare("UPDATE users SET username = ? WHERE id = ?");
                updateQuery.addBindValue(newUsername);
                updateQuery.addBindValue(userId);

                if (!updateQuery.exec()) {
                    throw std::runtime_error("更新用户名失败: " + updateQuery.lastError().text().toStdString());
                }

                // 4. 更新相关表中的用户名（如果有的话）
                // 例如：更新订单表中的用户名引用等

                // 提交事务
                QSqlDatabase::database().commit();

                reply.data["success"] = true;
                reply.data["message"] = "用户名修改成功";
                reply.data["new_username"] = newUsername;

                qDebug() << "用户名修改成功:" << oldUsername << "->" << newUsername;

            } catch (const std::exception &e) {
                // 回滚事务
                QSqlDatabase::database().rollback();

                reply.data["success"] = false;
                reply.data["message"] = QString("修改失败: %1").arg(e.what());
                qDebug() << "修改用户名失败:" << e.what();
            }

            networkManager.sendMessage(reply, client);
            break;
        }

        // =============== 新增：修改密码处理 ===============
        case CHANGE_PASSWORD_REQUEST: {  // 106
            QString username = message.data["username"].toString();
            QString oldPassword = message.data["old_password"].toString();
            QString newPassword = message.data["new_password"].toString();

            qDebug() << "=== 修改密码请求 ===";
            qDebug() << "用户名:" << username;

            NetworkMessage reply;
            reply.type = CHANGE_PASSWORD_RESPONSE;  // 107

            // 开始事务
            QSqlDatabase::database().transaction();

            try {
                // 1. 验证原密码
                QSqlQuery checkUser;
                checkUser.prepare("SELECT id, password FROM users WHERE username = ?");
                checkUser.addBindValue(username);

                if (!checkUser.exec() || !checkUser.next()) {
                    throw std::runtime_error("用户不存在");
                }

                int userId = checkUser.value("id").toInt();
                QString storedPassword = checkUser.value("password").toString();

                // 这里应该使用加密验证，为了简单起见直接比较
                if (storedPassword != oldPassword) {
                    throw std::runtime_error("原密码错误");
                }

                // 2. 检查新密码是否与原密码相同
                if (oldPassword == newPassword) {
                    throw std::runtime_error("新密码不能与原密码相同");
                }

                // 3. 检查密码长度
                if (newPassword.length() < 6 || newPassword.length() > 20) {
                    throw std::runtime_error("密码长度应在6-20个字符之间");
                }

                // 4. 更新密码
                QSqlQuery updateQuery;
                updateQuery.prepare("UPDATE users SET password = ? WHERE id = ?");
                updateQuery.addBindValue(newPassword);
                updateQuery.addBindValue(userId);

                if (!updateQuery.exec()) {
                    throw std::runtime_error("更新密码失败: " + updateQuery.lastError().text().toStdString());
                }

                // 提交事务
                QSqlDatabase::database().commit();

                reply.data["success"] = true;
                reply.data["message"] = "密码修改成功";

                qDebug() << "密码修改成功，用户:" << username;

            } catch (const std::exception &e) {
                // 回滚事务
                QSqlDatabase::database().rollback();

                reply.data["success"] = false;
                reply.data["message"] = QString("修改失败: %1").arg(e.what());
                qDebug() << "修改密码失败:" << e.what();
            }

            networkManager.sendMessage(reply, client);
            break;
        }
        default:
            qDebug() << "未知消息类型:" << message.type;
        }
    }

private:
    ServerNetworkManager networkManager;
};

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    TestServer server;
    server.start();

    return a.exec();
}

#include "main.moc"
