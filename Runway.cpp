#include <mutex>
#include "Runway.h"
#include "AirportToken.h"

/** A runway, essential for air travel. */

Runway::Runway(const int length) : length(length) {}

AirportToken Runway::checkReservation() {
	return reservationToken;
}

void Runway::assignReservation(const AirportToken token) {
	reservationToken = token;
}

Runway::RunwayState Runway::checkState() {
	return status;
}

bool Runway::reqRunway() {
	std::lock_guard<std::recursive_mutex> rwyLock(rwyMutex);
	if (status == RunwayState::Available) {
		status = RunwayState::Reserved;
		return true;
	}
	return false;
}

void Runway::freeRunway() {
	std::lock_guard<std::recursive_mutex> rwyLock(rwyMutex);
	status = RunwayState::Available;
}

void Runway::runwayOps(const AirportToken token) {
	if (token == reservationToken) {
		std::lock_guard<std::recursive_mutex> rwyLock(rwyMutex);
		status = RunwayState::InOperation;
	}
}
