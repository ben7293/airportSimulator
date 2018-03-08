#include <chrono>
#include <iostream>
#include <thread>
#include <vector>
#include <string>

#include "AirportToken.h"
#include "Runway.h"
#include "ParkingStand.h"
#include "Airport.h"


/**
* @defgroup Boilerplate
* Basic support for logging and random sleeps. Nothing exciting to see here.
* @{ */

/** Some simple thread-safe logging functionality. */
void Log() {}

static std::recursive_mutex s_mutex;

template<typename First, typename ...Rest>
void Log(First&& first, Rest&& ...rest) {
	std::lock_guard<std::recursive_mutex> lock(s_mutex);
	std::cout << std::forward<First>(first);
	Log(std::forward<Rest>(rest)...);
}

/** Produces a random integer within the [1..max] range. */
static int RandomInt(int maxSleep = 6) {
	const int minSleep = 1;
	return minSleep + (rand() % static_cast<int>(maxSleep - minSleep + 1));
}

/** @} */

/** This can be used to initialize the airport simulation object operation duration parameter
* with a value which is used by the testing code. */
static constexpr const int kOperationDurationSec = 5;

// A token is valid for 2 seconds
const std::chrono::seconds tokenValidity(2);

AirportToken tryRequestLandingToken(const std::string& id, Airport* airport) {
	tuple<Airport::AirportDirective, AirportToken> landingReqResult;

	do {
		this_thread::sleep_for(std::chrono::seconds(2));
		landingReqResult = airport->requestLanding(id);
	} while (get<0>(landingReqResult) != Airport::AirportDirective::Proceed);

	Log(id, " received a landing token.\n");
	return get<1>(landingReqResult);
}

AirportToken tryRequestTakeoffToken(const std::string& id, Airport* airport) {
	tuple<Airport::AirportDirective, AirportToken> takeoffReqResult;

	do {
		this_thread::sleep_for(std::chrono::seconds(2));
		takeoffReqResult = airport->requestTakeOff(id);
	} while (get<0>(takeoffReqResult) != Airport::AirportDirective::Proceed);

	Log(id, " received a take-off token.\n");
	return get<1>(takeoffReqResult);
}


int main() {

	Runway fourLeft(3682);
	Runway fourRight(2560);
	vector<Runway*> rwy;
	rwy.push_back(&fourLeft);
	rwy.push_back(&fourRight);
	ParkingStand pkg1;
	ParkingStand pkg2;
	ParkingStand pkg3;
	ParkingStand pkg4;
	vector<ParkingStand*> pkg;
	pkg.push_back(&pkg1);
	pkg.push_back(&pkg2);
	pkg.push_back(&pkg3);
	pkg.push_back(&pkg4);

	Airport* airport = new Airport{ rwy, pkg };

	// Now spin a number of threads simulating some aircrafts.
	vector<shared_ptr<thread>> aircrafts;
	
	for (int i = 0; i < 10; i++){
		aircrafts.push_back(make_shared<thread>([&airport, i](){
			std::string id{ "Aircraft " };
			id.append(to_string(i));
	
			for (;;)
			{

				/* Attempt calling into Airport::RequestLanding until landing token is successfully received. */
				{
					AirportToken landingToken = tryRequestLandingToken(id, airport);

					// Wait some random time. In some cases the token will expire, that's OK.
					this_thread::sleep_for(std::chrono::seconds{ RandomInt(3) });

					/* Attempt calling into Airport::PerformLanding and break out of the loop if it's successful. Repeat otherwise. */
					Airport::AirportOpsResult landingOpsResult = Airport::AirportOpsResult::InvalidParams;
					do {
						Log(id, " is landing.\n");
						landingOpsResult = airport->performLanding(landingToken);
						if (landingOpsResult == Airport::AirportOpsResult::ExpiredToken) {
							Log(id, " landing token expired. It will try again.\n");
							landingToken = tryRequestLandingToken(id, airport);
						}
					} while (landingOpsResult != Airport::AirportOpsResult::Success);
					
					if (landingOpsResult == Airport::AirportOpsResult::Success) {
						Log(id, " has landed.\n");
						break;
					}
				}

			}

			// Sleep for at least the time of landing operation plus some random time
			this_thread::sleep_for(std::chrono::seconds{ std::chrono::seconds{ kOperationDurationSec + RandomInt() } });

			for (;;) {
				/* Attempt calling into Airport::RequestTakeoff until take-off token is successfully received. */
				AirportToken takeoffToken = tryRequestTakeoffToken(id, airport);

				// Wait some random time. In some cases the token will expire, that's OK.
				this_thread::sleep_for(std::chrono::seconds{ RandomInt(3) });

				/* Attempt calling into Airport::PerformTakeoff and break out of the loop if it's successful. Repeat otherwise. */
				Airport::AirportOpsResult takeoffOpsResult = Airport::AirportOpsResult::InvalidParams;
				do {
					Log(id, " is taking off.\n");
					takeoffOpsResult = airport->performTakeOff(takeoffToken);
					if (takeoffOpsResult == Airport::AirportOpsResult::ExpiredToken) {
						Log(id, " take-off token expired. It will try again.\n");
						takeoffToken = tryRequestTakeoffToken(id, airport);
					}
				} while (takeoffOpsResult != Airport::AirportOpsResult::Success);
				
				if (takeoffOpsResult == Airport::AirportOpsResult::Success) {
					Log(id, " has taken off.\n");
					break;
				}

			}
	
	}));
	}
	
	for (auto aircraft : aircrafts) {
		aircraft->join();
	}

	return 0;
}