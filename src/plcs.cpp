// Activity 2: Parking Lot Control System with unlimited capacity.
//
// Usage: ./plcs [numCars=5] [seconds=0]
//   numCars = 1 gives the single-car version (3 threads in total).
//   seconds = 0 runs until the program is terminated (Ctrl+C).

#include <cstdlib>
#include <thread>
#include <vector>

#include "common.h"
#include "gate.h"

using namespace plcs;

static void car(int id, Gate &entry, Gate &exit) {
    std::mt19937 rng(id * 7919u);
    std::uniform_int_distribution<int> drive(200, 1000);
    const int parkMs = 500 + id * 400; // each car stays a different time

    while (!stopFlag().stopped()) {
        sleepMs(drive(rng));                 // driving around
        log(carName(id) + " arrives at the entry gate");
        if (!entry.claim(id) || !entry.passThrough(id)) break;

        log(carName(id) + " is parked for " + std::to_string(parkMs) + " ms");
        sleepMs(parkMs);

        log(carName(id) + " wants to leave");
        if (!exit.claim(id) || !exit.passThrough(id)) break;
        log(carName(id) + " has left the parking lot");
    }
}

int main(int argc, char *argv[]) {
    const int numCars = argc > 1 ? std::atoi(argv[1]) : 5;
    const int seconds = argc > 2 ? std::atoi(argv[2]) : 0;
    startTime();

    Gate entry("ENTRY gate");
    Gate exit("EXIT gate");

    std::thread entryThread(&Gate::run, &entry);
    std::thread exitThread(&Gate::run, &exit);

    std::vector<std::thread> cars;
    for (int i = 1; i <= numCars; ++i) cars.emplace_back(car, i, std::ref(entry), std::ref(exit));

    if (seconds > 0) {
        sleepMs(seconds * 1000);
        log("--- shutting down ---");
        stopFlag().stop();
        entry.stop();
        exit.stop();
    }

    for (auto &t : cars) t.join();
    entryThread.join();
    exitThread.join();
    return 0;
}
