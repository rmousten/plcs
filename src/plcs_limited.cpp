// Activity 3: Parking Lot Control System with limited capacity.
//
// Usage: ./plcs_limited [numCars=6] [capacity=2] [seconds=0]
//
// Lock order:
//   entering car: entry gate -> capacity mutex
//   leaving car:  capacity mutex -> exit gate
// No thread takes the locks in the opposite order, and the condition-variable
// wait in ParkingLot::enter() releases the capacity mutex, so no circular wait.

#include <cstdlib>
#include <thread>
#include <vector>

#include "common.h"
#include "gate.h"
#include "parking_lot.h"

using namespace plcs;

static void car(int id, Gate &entry, Gate &exit, ParkingLot &lot) {
    std::mt19937 rng(id * 7919u);
    std::uniform_int_distribution<int> drive(100, 600);
    const int parkMs = 800 + id * 300;

    while (!stopFlag().stopped()) {
        sleepMs(drive(rng));
        log(carName(id) + " arrives at the entry gate");

        if (!entry.claim(id)) break;     // 1. own the entry gate
        if (!lot.enter(id)) break;       // 2. capacity mutex: wait for a free space, free--
        if (!entry.passThrough(id)) break; // 3. gate opens, drive in

        log(carName(id) + " is parked for " + std::to_string(parkMs) + " ms");
        sleepMs(parkMs);

        log(carName(id) + " wants to leave");
        // capacity mutex -> exit gate: free++, pass the exit gate, notify a waiting car
        if (!lot.leaveThrough(id, exit)) break;
        log(carName(id) + " has left the parking lot");
    }
}

int main(int argc, char *argv[]) {
    const int numCars = argc > 1 ? std::atoi(argv[1]) : 6;
    const int capacity = argc > 2 ? std::atoi(argv[2]) : 2;
    const int seconds = argc > 3 ? std::atoi(argv[3]) : 0;
    startTime();

    Gate entry("ENTRY gate");
    Gate exit("EXIT gate");
    ParkingLot lot(capacity);
    log("Parking lot with " + std::to_string(capacity) + " spaces, " + std::to_string(numCars) + " cars");

    std::thread entryThread(&Gate::run, &entry);
    std::thread exitThread(&Gate::run, &exit);

    std::vector<std::thread> cars;
    for (int i = 1; i <= numCars; ++i)
        cars.emplace_back(car, i, std::ref(entry), std::ref(exit), std::ref(lot));

    if (seconds > 0) {
        sleepMs(seconds * 1000);
        log("--- shutting down ---");
        stopFlag().stop();
        lot.stop();
        entry.stop();
        exit.stop();
    }

    for (auto &t : cars) t.join();
    entryThread.join();
    exitThread.join();
    return 0;
}
