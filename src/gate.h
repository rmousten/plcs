#pragma once
// A bar gate (entry or exit) running in its own thread.
//
// Handshake between a car and the gate (all state protected by m_):
//   Car:  claim()       wait until no other car owns the gate, then own it (claimed_ = true)
//   Car:  passThrough() set requested_, wake gate, wait until open_
//   Gate: run()         wait for requested_, open (open_ = true), wake cars,
//                       wait for passed_, close and release the gate (claimed_ = false), wake cars
//   Car:  passThrough() drives through, sets passed_, wakes gate
//
// Two condition variables:
//   gateCv_ - only the gate thread waits here      -> notify_one() is enough
//   carCv_  - cars wait here for two different predicates (!claimed_ and open_)
//             -> notify_all() is needed, otherwise the wrong car could be woken
//                and the signal would be lost.

#include <condition_variable>
#include <mutex>
#include <string>

#include "common.h"

namespace plcs {

class Gate {
public:
    explicit Gate(std::string name) : name_(std::move(name)) {}

    // Called by a car: wait until the gate is free and reserve it.
    // Returns false if the system is shutting down.
    bool claim(int carId) {
        std::unique_lock<std::mutex> lk(m_);
        carCv_.wait(lk, [&] { return !claimed_ || stop_; });
        if (stop_) return false;
        claimed_ = true;
        currentCar_ = carId;
        return true;
    }

    // Called by the car that owns the gate: request opening, wait, drive through.
    bool passThrough(int carId) {
        std::unique_lock<std::mutex> lk(m_);
        requested_ = true;
        log(carName(carId) + " requests " + name_);
        gateCv_.notify_one();                            // wake the gate thread
        carCv_.wait(lk, [&] { return open_ || stop_; }); // wait for the bar to go up
        if (stop_) return false;
        log(carName(carId) + " drives through " + name_);
        passed_ = true;
        gateCv_.notify_one();                            // tell the gate we are through
        return true;
    }

    // Gate thread body.
    void run() {
        std::unique_lock<std::mutex> lk(m_);
        while (true) {
            // Gate stays closed until a car requests it.
            gateCv_.wait(lk, [&] { return requested_ || stop_; });
            if (stop_) break;

            open_ = true;
            log(name_ + " OPENS for " + carName(currentCar_));
            carCv_.notify_all();

            gateCv_.wait(lk, [&] { return passed_ || stop_; });
            if (stop_) break;

            open_ = false;
            requested_ = false;
            passed_ = false;
            claimed_ = false; // release the gate for the next car
            log(name_ + " CLOSES");
            carCv_.notify_all();
        }
        log(name_ + " thread stopped");
    }

    // Wake everybody so all threads can terminate.
    void stop() {
        std::lock_guard<std::mutex> lk(m_);
        stop_ = true;
        gateCv_.notify_all();
        carCv_.notify_all();
    }

private:
    std::string name_;
    std::mutex m_;
    std::condition_variable gateCv_;
    std::condition_variable carCv_;
    bool claimed_ = false;   // a car owns the gate
    bool requested_ = false; // owning car asks to open
    bool open_ = false;      // bar is up
    bool passed_ = false;    // car has driven through
    bool stop_ = false;
    int currentCar_ = -1;
};

} // namespace plcs
