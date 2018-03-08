#pragma once
#ifndef AIRPORT_TOKEN_H
#define AIRPORT_TOKEN_H

#include <string>
#include <chrono>

class Runway;
class ParkingStand;

class AirportToken {
public:
	AirportToken(std::string aircraftID = "", Runway* runwayID = nullptr, ParkingStand* parkingStandID = nullptr, std::chrono::seconds validity = std::chrono::seconds(0));
	bool hasTokenExpired() const;
	bool isTokenValid() const;
	bool operator==(AirportToken rhs) const;
	void fillToken(std::string aircraftID, Runway* runwayID, ParkingStand* parkingStandID, std::chrono::seconds validity);
	std::string getAircraftID() const;
	Runway* getRunwayID() const;
	ParkingStand* getParkingStandID() const;

private:
	std::string aircraftID;
	Runway* runwayID;
	ParkingStand* parkingStandID;
	std::chrono::system_clock::time_point expiry;
};


#endif