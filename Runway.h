#pragma once
#ifndef RUNWAY_H
#define RUNWAY_H

#include <mutex>
#include "AirportToken.h"

/** A runway, essential for air travel. */
class Runway {
public:
	enum RunwayState {
		InOperation,
		Reserved,
		Available
	};
	Runway(const int length);
	AirportToken checkReservation();
	RunwayState checkState();
	void runwayOps(const AirportToken token);
	friend class Airport;

private:
	bool reqRunway();
	void freeRunway();
	void assignReservation(const AirportToken token);
	std::recursive_mutex rwyMutex;
	const int length;
	RunwayState status = RunwayState::Available;
	AirportToken reservationToken;
};



#endif