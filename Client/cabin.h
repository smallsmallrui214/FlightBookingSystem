#ifndef CABIN_H
#define CABIN_H

#include <QString>
#include <QJsonObject>

class Cabin
{
public:
    Cabin();
    Cabin(int id, int flightId, const QString &cabinType,
          double price, int availableSeats, int totalSeats,
          const QString &baggageAllowance = "", const QString &amenities = "");

    QJsonObject toJson() const;
    static Cabin fromJson(const QJsonObject &json);

    // Getters
    int getId() const { return id; }
    int getFlightId() const { return flightId; }
    QString getCabinType() const { return cabinType; }
    double getPrice() const { return price; }
    int getAvailableSeats() const { return availableSeats; }
    int getTotalSeats() const { return totalSeats; }
    QString getBaggageAllowance() const { return baggageAllowance; }
    QString getAmenities() const { return amenities; }

private:
    int id;
    int flightId;
    QString cabinType;
    double price;
    int availableSeats;
    int totalSeats;
    QString baggageAllowance;
    QString amenities;
};

#endif // CABIN_H
