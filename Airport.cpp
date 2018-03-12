#include "Airport.h"
#include "ParkingStand.h"
#include "AirportToken.h"
#include "Runway.h"
#include <iostream>

static constexpr const int kOperationDurationSec = 5;
const std::chrono::seconds tokenValidity(2);

Airport::Airport(std::vector<Runway*> runways, std::vector<ParkingStand*> parkingStands) : runways(runways), parkingStands(parkingStands) {
	cleanUpThread = make_shared<thread>([=] {
		revokeExpiredTokens();
	});
}


void Airport::setTimerForOperation(const AirportToken token, const OpsType& opsType) {
	token.getRunwayID()->runwayOps(token);
	this_thread::sleep_for(chrono::seconds{ chrono::seconds{ kOperationDurationSec } });
	token.getRunwayID()->freeRunway();

	if (opsType == OpsType::Takeoff) {
		token.getParkingStandID()->freeParking();
	}
	else { // landing
		token.getParkingStandID()->parkingOps(token);
	}
}

void Airport::revokeToken(const AirportToken token) {
	std::lock_guard<std::recursive_mutex> aptLock(resourceMutex);
	// find matching token and free up resources if never used
	for (size_t i = 0; i < outstandingTokens.size(); ++i) {
		if (outstandingTokens[i] == token) {
			if (token.getRunwayID()->checkState() == Runway::RunwayState::Reserved &&
				token.getRunwayID()->checkReservation() == token) {
				token.getRunwayID()->freeRunway();
				if (token.getParkingStandID()->checkState() == ParkingStand::ParkingState::Reserved &&
					token.getParkingStandID()->checkReservation() == token) {
					token.getParkingStandID()->freeParking();
				}
			}
			outstandingTokens[i] = outstandingTokens[outstandingTokens.size() - 1];
			outstandingTokens.pop_back();
			break;
		}
	}
}

// Checks uniqueness of aircraft ID.
bool Airport::isAircraftIdentUnique(const string& id, Airport::OpsType opsType) {
	std::lock_guard<std::recursive_mutex> aptLock(resourceMutex);

	// For takeoff, if there is no outstading token, we assume it is unique
	for (size_t i = 0; i < outstandingTokens.size(); ++i) {
		if (outstandingTokens[i].getAircraftID() == id) return false;
	}

	// Additionally, for landing, if there is no aircraft with same ID on ground, we assume it is unique
	if (opsType == Airport::OpsType::Landing) {
		for (size_t j = 0; j < parkingStands.size(); ++j) {
			if ((parkingStands[j]->checkState() == ParkingStand::ParkingState::Occupied || parkingStands[j]->checkState() == ParkingStand::ParkingState::Reserved)
				&& (parkingStands[j]->checkReservation().getAircraftID() == id)) return false;
		}
	}
	return true;
}

Runway* Airport::reqRunway() {
	std::lock_guard<std::recursive_mutex> aptLock(resourceMutex);
	for (Runway* rwy : runways) {
		if (rwy->checkState() == Runway::RunwayState::Available) {
			if (rwy->reqRunway()) return rwy;
		}
	}
	return nullptr;
}

ParkingStand* Airport::reqParking() {
	// Checks list of parking stands for available parking stands and reserves it.
	std::lock_guard<std::recursive_mutex> aptLock(resourceMutex);
	for (ParkingStand* pkg : parkingStands) {
		if (pkg->checkState() == ParkingStand::ParkingState::Available) {
			if (pkg->reqParking()) return pkg;
		}
	}
	return nullptr;
}

ParkingStand* Airport::locateAircraftOnGround(const string& aircraftID) {
	// Returns first instance of a parking stand with reservation of a matching aircraft ID.
	for (ParkingStand* pkg : parkingStands) {
		if (pkg->checkReservation().getAircraftID() == aircraftID) {
			return pkg;
		}
	}
	return nullptr;
}

tuple<Airport::AirportDirective, AirportToken> Airport::requestLanding(const string& aircraftID) {
	std::lock_guard<std::recursive_mutex> aptLock(resourceMutex);
	AirportToken token;

	if (!isAircraftIdentUnique(aircraftID, OpsType::Landing)) return make_tuple(AirportDirective::Hold, token);

	// request resources
	Runway* runwayID = reqRunway();
	ParkingStand* parkingStandID = reqParking();

	// fill token info
	if (runwayID == nullptr) {
		if (parkingStandID != nullptr) parkingStandID->freeParking();
		return make_tuple(AirportDirective::Hold, token);
	}
	else if (parkingStandID == nullptr) {
		if (runwayID != nullptr) runwayID->freeRunway();
		return make_tuple(AirportDirective::Hold, token);
	}
	else {
		token = AirportToken(aircraftID, runwayID, parkingStandID, tokenValidity);
		parkingStandID->assignReservation(token);
		runwayID->assignReservation(token);
		outstandingTokens.push_back(token);
		return make_tuple(AirportDirective::Proceed, token);
	}
}

Airport::AirportOpsResult Airport::performLanding(const AirportToken token) {
	// Is token valid?
	if (!token.isTokenValid()) return AirportOpsResult::InvalidParams;
	else if (token.hasTokenExpired()) {
		revokeToken(token);
		return AirportOpsResult::ExpiredToken;
	}
	else {
		thread landingOps(setTimerForOperation, token, OpsType::Landing);
		landingOps.detach();
		return AirportOpsResult::Success;
	}
}

tuple<Airport::AirportDirective, AirportToken> Airport::requestTakeOff(const string& aircraftID) {
	std::lock_guard<std::recursive_mutex> aptLock(resourceMutex);
	AirportToken token;

	if (!isAircraftIdentUnique(aircraftID, OpsType::Takeoff)) return make_tuple(AirportDirective::Hold, token);

	// request runway
	Runway* runwayID = reqRunway();

	// locate parking stand
	ParkingStand* parkingStandID = locateAircraftOnGround(aircraftID);

	// fill token info
	if (parkingStandID == nullptr) return make_tuple(AirportDirective::Hold, token);
	if (runwayID == nullptr) return make_tuple(AirportDirective::Hold, token);

	token = AirportToken(aircraftID, runwayID, parkingStandID, tokenValidity);
	parkingStandID->assignReservation(token);
	runwayID->assignReservation(token);
	outstandingTokens.push_back(token);
	return make_tuple(AirportDirective::Proceed, token);
}

Airport::AirportOpsResult Airport::performTakeOff(const AirportToken token) {
	// Is token valid?
	if (!token.isTokenValid()) return AirportOpsResult::InvalidParams;
	else if (token.hasTokenExpired()) {
		revokeToken(token);
		return AirportOpsResult::ExpiredToken;
	}
	else {
		thread landingOps(setTimerForOperation, token, OpsType::Takeoff);
		landingOps.detach();
		return AirportOpsResult::Success;
	}
}

// Checks validity of tokens every 5 seconds for infinite amount of time.
void Airport::revokeExpiredTokens() {
	while (!destroyed) {
		this_thread::sleep_for(chrono::seconds(5));
		std::vector<AirportToken> removeTokenList;
		std::lock_guard<std::recursive_mutex> aptLock(resourceMutex);
		for (size_t i = 0; i < outstandingTokens.size(); ++i) {
			if (outstandingTokens[i].hasTokenExpired()) {
				removeTokenList.push_back(outstandingTokens[i]);
			}
		}
		for (const AirportToken& token : removeTokenList) {
			revokeToken(token);
		}
	}
}

Airport::~Airport() {
	destroyed = true;
	cleanUpThread->join();
}