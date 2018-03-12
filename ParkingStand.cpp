#include "ParkingStand.h"
#include "AirportToken.h"


/** Parking stand. Useful whether you're in a 747-800 or a station wagon. */

AirportToken ParkingStand::checkReservation() const {
	return reservationToken;
}


ParkingStand::ParkingState ParkingStand::checkState() const {
	return status;
}

void ParkingStand::parkingOps(const AirportToken& token) {
	if (token == reservationToken) {
		lock_guard<recursive_mutex> pkgLock(pkgMutex);
		status = ParkingState::Occupied;
	}
}

bool ParkingStand::reqParking() {
	lock_guard<recursive_mutex> pkgLock(pkgMutex);
	if (status == ParkingState::Available) {
		status = ParkingState::Reserved;
		return true;
	}
	return false;
}

void ParkingStand::freeParking() {
	lock_guard<recursive_mutex> pkgLock(pkgMutex);
	status = ParkingState::Available;
	reservationToken = AirportToken();
}

void ParkingStand::assignReservation(AirportToken token) {
	reservationToken = token;
}


