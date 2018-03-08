#include "AirportToken.h"
class ParkingStand;
class Runway;

AirportToken::AirportToken(std::string aircraftID, Runway* runwayID, ParkingStand* parkingStandID, std::chrono::seconds validity) :
	aircraftID(aircraftID), runwayID(runwayID), parkingStandID(parkingStandID), expiry(std::chrono::system_clock::now() + validity) {}

bool AirportToken::hasTokenExpired() const {
	return std::chrono::system_clock::now() > expiry;
}

bool AirportToken::isTokenValid() const {
	return aircraftID != "" && parkingStandID != nullptr && runwayID != nullptr;
}

bool AirportToken::operator==(AirportToken rhs) const {
	return
		aircraftID == rhs.aircraftID &&
		runwayID == rhs.runwayID &&
		parkingStandID == rhs.parkingStandID &&
		expiry == rhs.expiry;
}

void AirportToken::fillToken(std::string aircraftID, Runway* runwayID, ParkingStand* parkingStandID, std::chrono::seconds validity) {
	this->aircraftID = aircraftID;
	this->runwayID = runwayID;
	this->parkingStandID = parkingStandID;
	this->expiry = std::chrono::system_clock::now() + validity;
}

std::string AirportToken::getAircraftID() const {
	return aircraftID;
}

Runway* AirportToken::getRunwayID() const {
	return runwayID;
}

ParkingStand* AirportToken::getParkingStandID() const {
	return parkingStandID;
}
