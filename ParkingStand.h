#pragma once
#ifndef PARKING_STAND_H
#define PARKING_STAND_H

#include <mutex>
using namespace std;

#include "AirportToken.h"


/** Parking stand. Useful whether you're in a 747-800 or a station wagon. */
class ParkingStand {
public:
	enum ParkingState {
		Occupied,
		Reserved,
		Available
	};
	AirportToken checkReservation() const;
	void assignReservation(AirportToken token);
	ParkingState checkState() const;
	bool reqParking();
	void freeParking();
	void parkingOps(const AirportToken& token);

private:
	recursive_mutex pkgMutex;
	ParkingState status = ParkingState::Available;
	AirportToken reservationToken;
};



#endif