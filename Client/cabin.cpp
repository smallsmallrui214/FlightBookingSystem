#include "cabin.h"

Cabin::Cabin()
    : id(-1), flightId(-1), price(0), availableSeats(0), totalSeats(0)
{
}

Cabin::Cabin(int id, int flightId, const QString &cabinType,
             double price, int availableSeats, int totalSeats,
             const QString &baggageAllowance, const QString &amenities)
    : id(id), flightId(flightId), cabinType(cabinType),
    price(price), availableSeats(availableSeats), totalSeats(totalSeats),
    baggageAllowance(baggageAllowance), amenities(amenities)
{
}

QJsonObject Cabin::toJson() const
{
    QJsonObject obj;
    obj["id"] = id;
    obj["flight_id"] = flightId;
    obj["cabin_type"] = cabinType;
    obj["price"] = price;
    obj["available_seats"] = availableSeats;
    obj["total_seats"] = totalSeats;
    obj["baggage_allowance"] = baggageAllowance;
    obj["amenities"] = amenities;
    return obj;
}

Cabin Cabin::fromJson(const QJsonObject &json)
{
    Cabin cabin;
    cabin.id = json["id"].toInt();
    cabin.flightId = json["flight_id"].toInt();
    cabin.cabinType = json["cabin_type"].toString();
    cabin.price = json["price"].toDouble();
    cabin.availableSeats = json["available_seats"].toInt();
    cabin.totalSeats = json["total_seats"].toInt();
    cabin.baggageAllowance = json["baggage_allowance"].toString();
    cabin.amenities = json["amenities"].toString();
    return cabin;
}
