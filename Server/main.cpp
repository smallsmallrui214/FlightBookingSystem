#include <QCoreApplication>
#include <QDebug>
#include <QJsonArray>
#include <QRandomGenerator>
#include <QTime>
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

            qDebug() << "=== 航班搜索请求 ===";
            qDebug() << "出发城市:" << departureCity;
            qDebug() << "到达城市:" << arrivalCity;
            qDebug() << "日期:" << date;
            qDebug() << "排序:" << sortBy << (sortAsc ? "升序" : "降序");

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

            qDebug() << "执行的SQL:" << sql;

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
                qDebug() << "SQL执行成功";
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
                    qDebug() << "找到航班:" << flight.getFlightNumber() << flight.getDepartureCity() << "->" << flight.getArrivalCity();
                }
                reply.data["success"] = true;
                reply.data["flights"] = flightsArray;
                qDebug() << "返回航班数据:" << flightsArray.size() << "条";
            } else {
                qDebug() << "SQL执行失败:" << query.lastError().text();
                reply.data["success"] = false;
                reply.data["message"] = "查询失败: " + query.lastError().text();
            }

            networkManager.sendMessage(reply, client);
            qDebug() << "=== 搜索处理完成 ===";
            break;
        }
        case CABIN_SEARCH_REQUEST: {
            int flightId = message.data["flight_id"].toInt();

            qDebug() << "=== 舱位查询请求 ===";
            qDebug() << "航班ID:" << flightId;

            QSqlQuery query;
            query.prepare("SELECT * FROM cabins WHERE flight_id = ? ORDER BY price ASC");
            query.bindValue(0, flightId);

            NetworkMessage reply;
            reply.type = CABIN_SEARCH_RESPONSE;

            if (query.exec()) {
                QJsonArray cabinsArray;
                int count = 0;
                while (query.next()) {
                    count++;
                    Cabin cabin(
                        query.value("id").toInt(),
                        query.value("flight_id").toInt(),
                        query.value("cabin_type").toString(),
                        query.value("price").toDouble(),
                        query.value("available_seats").toInt(),
                        query.value("total_seats").toInt(),
                        query.value("baggage_allowance").toString(),
                        query.value("amenities").toString()
                        );
                    cabinsArray.append(cabin.toJson());
                    qDebug() << "找到舱位:" << cabin.getCabinType() << "价格:" << cabin.getPrice();
                }
                reply.data["success"] = true;
                reply.data["cabins"] = cabinsArray;
                qDebug() << "返回舱位数据:" << cabinsArray.size() << "条";
            } else {
                qDebug() << "舱位查询失败:" << query.lastError().text();
                reply.data["success"] = false;
                reply.data["message"] = "查询舱位信息失败: " + query.lastError().text();
            }

            networkManager.sendMessage(reply, client);
            break;
        }
        case BOOKING_REQUEST: {
            int flightId = message.data["flight_id"].toInt();
            int cabinId = message.data["cabin_id"].toInt();
            QString passengerName = message.data["passenger_name"].toString();
            QString passengerId = message.data["passenger_id"].toString();
            QString passengerPhone = message.data["passenger_phone"].toString();
            double totalPrice = message.data["total_price"].toDouble();

            int userId = message.data["user_id"].toInt();
            if (userId <= 0) {
                userId = 1;
            }

            qDebug() << "=== 预订请求 ===";
            qDebug() << "航班ID:" << flightId;
            qDebug() << "舱位ID:" << cabinId;
            qDebug() << "乘客:" << passengerName;
            qDebug() << "用户ID:" << userId;

            NetworkMessage reply;
            reply.type = BOOKING_RESPONSE;

            // 开始事务
            QSqlDatabase::database().transaction();

            try {
                // 1. 检查舱位可用性
                QSqlQuery checkQuery;
                checkQuery.prepare("SELECT available_seats FROM cabins WHERE id = ?");
                checkQuery.bindValue(0, cabinId);

                if (!checkQuery.exec() || !checkQuery.next()) {
                    throw std::runtime_error("舱位不存在");
                }

                int availableSeats = checkQuery.value(0).toInt();
                if (availableSeats <= 0) {
                    throw std::runtime_error("该舱位已无可用座位");
                }

                // 2. 生成订单号 - 使用QRandomGenerator
                QString dateStr = QDateTime::currentDateTime().toString("yyyyMMdd");
                int randomNum = QRandomGenerator::global()->bounded(10000);
                QString bookingNumber = "BK" + dateStr + QString("%1").arg(randomNum, 4, 10, QChar('0'));

                // 3. 创建预订
                QSqlQuery insertQuery;
                insertQuery.prepare(
                    "INSERT INTO bookings (booking_number, user_id, flight_id, cabin_id, "
                    "passenger_name, passenger_id, passenger_phone, total_price, "
                    "departure_city, arrival_city, departure_time, arrival_time, "
                    "flight_number, airline, travel_date) "
                    "SELECT ?, ?, f.id, ?, ?, ?, ?, ?, "
                    "f.departure_city, f.arrival_city, f.departure_time, f.arrival_time, "
                    "f.flight_number, f.airline, DATE(f.departure_time) "
                    "FROM flights f WHERE f.id = ?"
                    );

                insertQuery.bindValue(0, bookingNumber);
                insertQuery.bindValue(1, userId);
                insertQuery.bindValue(2, cabinId);
                insertQuery.bindValue(3, passengerName);
                insertQuery.bindValue(4, passengerId);
                insertQuery.bindValue(5, passengerPhone);
                insertQuery.bindValue(6, totalPrice);
                insertQuery.bindValue(7, flightId);

                if (!insertQuery.exec()) {
                    throw std::runtime_error("创建订单失败: " + insertQuery.lastError().text().toStdString());
                }

                int bookingId = insertQuery.lastInsertId().toInt();

                // 4. 更新舱位可用座位数
                QSqlQuery updateCabinQuery;
                updateCabinQuery.prepare("UPDATE cabins SET available_seats = available_seats - 1 WHERE id = ?");
                updateCabinQuery.bindValue(0, cabinId);

                if (!updateCabinQuery.exec()) {
                    throw std::runtime_error("更新舱位座位数失败");
                }

                // 5. 更新航班可用座位数
                QSqlQuery updateFlightQuery;
                updateFlightQuery.prepare("UPDATE flights SET available_seats = available_seats - 1 WHERE id = ?");
                updateFlightQuery.bindValue(0, flightId);

                if (!updateFlightQuery.exec()) {
                    throw std::runtime_error("更新航班座位数失败");
                }

                // 提交事务
                QSqlDatabase::database().commit();

                reply.data["success"] = true;
                reply.data["message"] = "预订成功";
                reply.data["booking_number"] = bookingNumber;
                reply.data["booking_id"] = bookingId;
                qDebug() << "预订成功，订单号:" << bookingNumber << "订单ID:" << bookingId;

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
