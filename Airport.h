#pragma once
#ifndef AIRPORT_H
#define AIRPORT_H

#include <vector>
#include <mutex>

#include "Runway.h"
#include "ParkingStand.h"
#include "AirportToken.h"

/** Simulation of an airport. Tiny preview of the headaches that come with the real thing. */
class Airport {
public:
	Airport(std::vector<Runway*> runways, std::vector<ParkingStand*> parkingStands);
	enum AirportDirective {
		Hold,
		Proceed
	};
	enum AirportOpsResult {
		Success,
		InvalidParams,
		ExpiredToken
	};
	enum OpsType {
		Takeoff,
		Landing
	};

	static void setTimerForOperation(const AirportToken token, const OpsType& opsType);
	void revokeToken(const AirportToken token);
	bool isAircraftIdentUnique(const string& id, Airport::OpsType opsType);
	Runway* reqRunway();
	ParkingStand* reqParking();
	ParkingStand* locateAircraftOnGround(const string& aircraftID);
	tuple<AirportDirective, AirportToken> requestLanding(const string& aircraftID);
	AirportOpsResult performLanding(const AirportToken token);
	tuple<AirportDirective, AirportToken> requestTakeOff(const string& aircraftID);
	AirportOpsResult performTakeOff(const AirportToken token);
	void revokeExpiredTokens();
	~Airport();

private:
	std::recursive_mutex resourceMutex;
	std::vector<Runway*> runways;
	std::vector<ParkingStand*> parkingStands;
	std::vector<AirportToken> outstandingTokens;
	shared_ptr<thread> cleanUpThread;
	bool destroyed = false;
};



#endif