#include <QCoreApplication>
#include <QDebug>
#include <QJsonArray>  // 添加这个头文件
#include "servernetworkmanager.h"
#include "databasemanager.h"
#include "../Common/flight.h"  // 添加Flight头文件

class TestServer : public QObject
{
    Q_OBJECT

public:
    TestServer() {
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

                // 更新最后登录时间
                bool updateSuccess = db.updateLastLogin(username);
                qDebug() << "更新最后登录时间结果:" << (updateSuccess ? "成功" : "失败");

                // 获取更新后的最后登录时间
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

            // 打印所有接收到的数据
            qDebug() << "所有接收到的数据:" << message.data;

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
                qDebug() << "✅ 添加航空公司筛选条件:" << airline;
            } else {
                qDebug() << "❌ 没有航空公司筛选条件";
            }

            // 排序
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
                qDebug() << "参数" << paramIndex << ":" << departureCity;
            }
            if (!arrivalCity.isEmpty()) {
                query.bindValue(paramIndex++, arrivalCity);
                qDebug() << "参数" << paramIndex << ":" << arrivalCity;
            }
            if (!date.isEmpty()) {
                query.bindValue(paramIndex++, date);
                qDebug() << "参数" << paramIndex << ":" << date;
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

                if (count == 0) {
                    qDebug() << "警告: 查询返回0条记录";
                    // 测试数据库连接和表数据
                    QSqlQuery testQuery("SELECT COUNT(*) as total FROM flights");
                    if (testQuery.exec() && testQuery.next()) {
                        qDebug() << "数据库中共有航班记录:" << testQuery.value("total").toInt() << "条";
                    }
                }
            } else {
                qDebug() << "SQL执行失败:" << query.lastError().text();
                reply.data["success"] = false;
                reply.data["message"] = "查询失败: " + query.lastError().text();
            }

            networkManager.sendMessage(reply, client);
            qDebug() << "=== 搜索处理完成 ===";
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
