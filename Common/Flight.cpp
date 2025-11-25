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
    return flight;
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
