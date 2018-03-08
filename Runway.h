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

	void assignReservation(const AirportToken token);

	RunwayState checkState();
	bool reqRunway();
	void freeRunway();

	void runwayOps(const AirportToken token);

private:
	std::recursive_mutex rwyMutex;
	const int length;
	RunwayState status = RunwayState::Available;
	AirportToken reservationToken;
};



#endif