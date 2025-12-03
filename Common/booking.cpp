#include "booking.h"
#include <QDateTime>
#include <QRandomGenerator>

Booking::Booking()
    : id(-1), userId(-1), flightId(-1), cabinId(-1), totalPrice(0)
{
}

Booking::Booking(int id, int userId, int flightId, int cabinId,
                 const QString &passengerName, const QString &passengerId,
                 const QString &passengerPhone, double totalPrice,
                 const QDateTime &bookingTime, const QString &status, const QString &seatNumber)
    : id(id), userId(userId), flightId(flightId), cabinId(cabinId),
    passengerName(passengerName), passengerId(passengerId),
    passengerPhone(passengerPhone), totalPrice(totalPrice),
    bookingTime(bookingTime), status(status), seatNumber(seatNumber)
{
}

QJsonObject Booking::toJson() const
{
    QJsonObject obj;
    obj["id"] = id;
    obj["user_id"] = userId;
    obj["flight_id"] = flightId;
    obj["cabin_id"] = cabinId;
    obj["passenger_name"] = passengerName;
    obj["passenger_id"] = passengerId;
    obj["passenger_phone"] = passengerPhone;
    obj["total_price"] = totalPrice;
    obj["booking_time"] = bookingTime.toString(Qt::ISODate);
    obj["status"] = status;
    obj["seat_number"] = seatNumber;
    return obj;
}

Booking Booking::fromJson(const QJsonObject &json)
{
    Booking booking;
    booking.id = json["id"].toInt();
    booking.userId = json["user_id"].toInt();
    booking.flightId = json["flight_id"].toInt();
    booking.cabinId = json["cabin_id"].toInt();
    booking.passengerName = json["passenger_name"].toString();
    booking.passengerId = json["passenger_id"].toString();
    booking.passengerPhone = json["passenger_phone"].toString();
    booking.totalPrice = json["total_price"].toDouble();
    booking.bookingTime = QDateTime::fromString(json["booking_time"].toString(), Qt::ISODate);
    booking.status = json["status"].toString();
    booking.seatNumber = json["seat_number"].toString();
    return booking;
}

QString Booking::generateBookingNumber() const
{
    QString dateStr = QDateTime::currentDateTime().toString("yyyyMMdd");
    int randomNum = QRandomGenerator::global()->bounded(10000);
    return "BK" + dateStr + QString("%1").arg(randomNum, 4, 10, QChar('0'));
}
