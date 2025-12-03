#ifndef BOOKING_H
#define BOOKING_H

#include <QString>
#include <QDateTime>
#include <QJsonObject>

class Booking
{
public:
    Booking();
    Booking(int id, int userId, int flightId, int cabinId,
            const QString &passengerName, const QString &passengerId,
            const QString &passengerPhone, double totalPrice,
            const QDateTime &bookingTime = QDateTime::currentDateTime(),
            const QString &status = "已预订", const QString &seatNumber = "");

    QJsonObject toJson() const;
    static Booking fromJson(const QJsonObject &json);

    // Getters
    int getId() const { return id; }
    int getUserId() const { return userId; }
    int getFlightId() const { return flightId; }
    int getCabinId() const { return cabinId; }
    QString getPassengerName() const { return passengerName; }
    QString getPassengerId() const { return passengerId; }
    QString getPassengerPhone() const { return passengerPhone; }
    double getTotalPrice() const { return totalPrice; }
    QDateTime getBookingTime() const { return bookingTime; }
    QString getStatus() const { return status; }
    QString getSeatNumber() const { return seatNumber; }

    // 生成订单号
    QString generateBookingNumber() const;

private:
    int id;
    int userId;
    int flightId;
    int cabinId;
    QString passengerName;
    QString passengerId;
    QString passengerPhone;
    double totalPrice;
    QDateTime bookingTime;
    QString status;
    QString seatNumber;
};

#endif // BOOKING_H
