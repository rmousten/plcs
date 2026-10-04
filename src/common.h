#pragma once
// Shared helpers: thread-safe logging, a global stop flag and random sleeps.
// Only std::mutex and std::condition_variable are used (no atomics/semaphores).

#include <chrono>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <random>
#include <sstream>
#include <string>
#include <thread>

namespace plcs {

using Clock = std::chrono::steady_clock;

inline Clock::time_point startTime() {
    static const Clock::time_point t0 = Clock::now();
    return t0;
}

// Thread-safe printing with a timestamp in ms since program start.
inline void log(const std::string &msg) {
    static std::mutex logMutex;
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - startTime()).count();
    std::lock_guard<std::mutex> lk(logMutex);
    std::cout << "[" << std::setw(6) << ms << " ms] " << msg << std::endl;
}

// Global "keep running" flag, protected by a mutex.
class StopFlag {
public:
    void stop() {
        std::lock_guard<std::mutex> lk(m_);
        stopped_ = true;
    }
    bool stopped() {
        std::lock_guard<std::mutex> lk(m_);
        return stopped_;
    }
private:
    std::mutex m_;
    bool stopped_ = false;
};

inline StopFlag &stopFlag() {
    static StopFlag flag;
    return flag;
}

inline void sleepMs(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

inline std::string carName(int id) {
    std::ostringstream os;
    os << "Car " << id;
    return os.str();
}

} // namespace plcs
