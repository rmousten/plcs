#pragma once
// Monitor guarding the number of free parking spaces (Activity 3).
//
//   enter(): lock capacity mutex, wait on notFull_ while free_ == 0, then free_--
//   leaveThrough(): lock capacity mutex, free_++, pass the exit gate while still
//                   holding the capacity mutex, unlock, notify_one()
//
// Lock order: entering car = entry gate -> capacity mutex,
//             leaving car  = capacity mutex -> exit gate.
// No thread ever takes them the other way round, so there is no circular wait.
// The wait in enter() releases the capacity mutex while blocked, so a leaving
// car can always get it. Because the capacity mutex is held until the car is
// physically out, a waiting car cannot drive in before the space is really free.
// Only the car that currently owns the entry gate can be waiting in enter(),
// so notify_one() is sufficient.

#include <cassert>
#include <condition_variable>
#include <mutex>

#include "common.h"
#include "gate.h"

namespace plcs {

class ParkingLot {
public:
    explicit ParkingLot(int capacity) : capacity_(capacity), free_(capacity) {}

    bool enter(int carId) {
        std::unique_lock<std::mutex> lk(m_);
        if (free_ == 0) log(carName(carId) + " waits: parking lot is FULL");
        notFull_.wait(lk, [&] { return free_ > 0 || stop_; });
        if (stop_) return false;
        --free_;
        assert(free_ >= 0 && free_ <= capacity_);
        log(carName(carId) + " reserves a space (free " + std::to_string(free_) + "/" +
            std::to_string(capacity_) + ")");
        return true;
    }

    bool leaveThrough(int carId, Gate &exitGate) {
        bool ok;
        {
            std::unique_lock<std::mutex> lk(m_);           // 1. capacity mutex
            ++free_;
            assert(free_ <= capacity_);
            log(carName(carId) + " frees its space (free " + std::to_string(free_) + "/" +
                std::to_string(capacity_) + ")");
            ok = exitGate.claim(carId) && exitGate.passThrough(carId); // 2. exit gate
        }                                                    // capacity mutex released
        notFull_.notify_one();                               // wake the waiting car (if any)
        return ok;
    }

    void stop() {
        std::lock_guard<std::mutex> lk(m_);
        stop_ = true;
        notFull_.notify_all();
    }

private:
    std::mutex m_;
    std::condition_variable notFull_;
    const int capacity_;
    int free_;
    bool stop_ = false;
};

} // namespace plcs
