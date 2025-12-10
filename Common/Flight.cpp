#include "flight.h"
#include <QTime>

Flight::Flight()
    : id(-1), durationMinutes(0), price(0), totalSeats(0), availableSeats(0)
{
}

Flight::Flight(int id, const QString &flightNumber, const QString &airline,
               const QString &departureCity, const QString &arrivalCity,
               const QDateTime &departureTime, const QDateTime &arrivalTime,
               int durationMinutes, double price, int totalSeats,
               int availableSeats, const QString &aircraftType, const QString &status)
    : id(id), flightNumber(flightNumber), airline(airline),
    departureCity(departureCity), arrivalCity(arrivalCity),
    departureTime(departureTime), arrivalTime(arrivalTime),
    durationMinutes(durationMinutes), price(price), totalSeats(totalSeats),
    availableSeats(availableSeats), aircraftType(aircraftType), status(status)
{
}

QJsonObject Flight::toJson() const
{
    QJsonObject obj;
    obj["id"] = id;
    obj["flight_number"] = flightNumber;
    obj["airline"] = airline;
    obj["departure_city"] = departureCity;
    obj["arrival_city"] = arrivalCity;
    obj["departure_time"] = departureTime.toString(Qt::ISODate);
    obj["arrival_time"] = arrivalTime.toString(Qt::ISODate);
    obj["duration_minutes"] = durationMinutes;
    obj["price"] = price;
    obj["total_seats"] = totalSeats;
    obj["available_seats"] = availableSeats;
    obj["aircraft_type"] = aircraftType;
    obj["status"] = status;

    obj["departure_airport"] = departureAirport;
    obj["arrival_airport"] = arrivalAirport;
    return obj;
}

Flight Flight::fromJson(const QJsonObject &json)
{
    Flight flight;
    flight.id = json["id"].toInt();
    flight.flightNumber = json["flight_number"].toString();
    flight.airline = json["airline"].toString();
    flight.departureCity = json["departure_city"].toString();
    flight.arrivalCity = json["arrival_city"].toString();
    flight.departureTime = QDateTime::fromString(json["departure_time"].toString(), Qt::ISODate);
    flight.arrivalTime = QDateTime::fromString(json["arrival_time"].toString(), Qt::ISODate);
    flight.durationMinutes = json["duration_minutes"].toInt();
    flight.price = json["price"].toDouble();
    flight.totalSeats = json["total_seats"].toInt();
    flight.availableSeats = json["available_seats"].toInt();
    flight.aircraftType = json["aircraft_type"].toString();
    flight.status = json["status"].toString();

    // 添加机场信息解析
    flight.departureAirport = json["departure_airport"].toString();
    flight.arrivalAirport = json["arrival_airport"].toString();

    // 如果json中没有机场字段，根据城市设置默认机场
    if (flight.departureAirport.isEmpty()) {
        flight.departureAirport = getDefaultAirport(flight.departureCity);
    }
    if (flight.arrivalAirport.isEmpty()) {
        flight.arrivalAirport = getDefaultAirport(flight.arrivalCity);
    }
    return flight;
}


QString Flight::getDefaultAirport(const QString &city)
{
    static QMap<QString, QString> cityToAirport = {
        {"北京", "首都国际机场 T3"},
        {"上海", "浦东国际机场 T2"},
        {"广州", "白云国际机场 T1"},
        {"深圳", "宝安国际机场 T3"},
        {"成都", "双流国际机场 T2"},
        {"重庆", "江北国际机场 T3"},
        {"杭州", "萧山国际机场 T4"},
        {"南京", "禄口国际机场 T1"},
        {"武汉", "天河国际机场 T3"},
        {"西安", "咸阳国际机场 T2"},
        {"昆明", "长水国际机场"},
        {"厦门", "高崎国际机场 T3"},
        {"青岛", "流亭国际机场"},
        {"大连", "周水子国际机场"},
        {"沈阳", "桃仙国际机场 T3"},
        {"哈尔滨", "太平国际机场 T2"},
        {"乌鲁木齐", "地窝堡国际机场 T3"},
        {"拉萨", "贡嘎国际机场"},
        {"海口", "美兰国际机场 T2"},
        {"三亚", "凤凰国际机场"},
        {"宜宾", "五粮液机场"},
        {"绵阳", "南郊机场"},
        {"泸州", "云龙机场"},
        {"南充", "高坪机场"},
        {"西昌", "青山机场"},
        {"广元", "盘龙机场"},
        {"达州", "河市机场"},
        {"攀枝花", "保安营机场"}
    };

    QString airport = cityToAirport.value(city, "");
    if (airport.isEmpty()) {
        return city + "机场";
    }
    return airport;
}

QString Flight::getDurationString() const
{
    int hours = durationMinutes / 60;
    int minutes = durationMinutes % 60;
    return QString("%1h%2m").arg(hours).arg(minutes, 2, 10, QChar('0'));
}

QString Flight::getDepartureDate() const
{
    return departureTime.toString("MM月dd日");
}

QString Flight::getDepartureTimeString() const
{
    return departureTime.toString("hh:mm");
}

QString Flight::getArrivalTimeString() const
{
    return arrivalTime.toString("hh:mm");
}
