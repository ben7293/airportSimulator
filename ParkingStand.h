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
	ParkingState checkState() const;
	void parkingOps(const AirportToken& token);
	friend class Airport;

private:
	bool reqParking();
	void freeParking();
	void assignReservation(AirportToken token);
	recursive_mutex pkgMutex;
	ParkingState status = ParkingState::Available;
	AirportToken reservationToken;
};



#endif