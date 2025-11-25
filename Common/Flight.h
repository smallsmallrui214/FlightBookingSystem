#ifndef FLIGHT_H
#define FLIGHT_H

#include <QString>
#include <QDateTime>
#include <QJsonObject>

class Flight
{
public:
    Flight();
    Flight(int id, const QString &flightNumber, const QString &airline,
           const QString &departureCity, const QString &arrivalCity,
           const QDateTime &departureTime, const QDateTime &arrivalTime,
           int durationMinutes, double price, int totalSeats,
           int availableSeats, const QString &aircraftType, const QString &status);

    // 序列化方法
    QJsonObject toJson() const;
    static Flight fromJson(const QJsonObject &json);

    // Getters
    int getId() const { return id; }
    QString getFlightNumber() const { return flightNumber; }
    QString getAirline() const { return airline; }
    QString getDepartureCity() const { return departureCity; }
    QString getArrivalCity() const { return arrivalCity; }
    QDateTime getDepartureTime() const { return departureTime; }
    QDateTime getArrivalTime() const { return arrivalTime; }
    int getDurationMinutes() const { return durationMinutes; }
    double getPrice() const { return price; }
    int getTotalSeats() const { return totalSeats; }
    int getAvailableSeats() const { return availableSeats; }
    QString getAircraftType() const { return aircraftType; }
    QString getStatus() const { return status; }

    // 计算属性
    QString getDurationString() const;
    QString getDepartureDate() const;
    QString getDepartureTimeString() const;
    QString getArrivalTimeString() const;

private:
    int id;
    QString flightNumber;
    QString airline;
    QString departureCity;
    QString arrivalCity;
    QDateTime departureTime;
    QDateTime arrivalTime;
    int durationMinutes;
    double price;
    int totalSeats;
    int availableSeats;
    QString aircraftType;
    QString status;
};

#endif // FLIGHT_H
