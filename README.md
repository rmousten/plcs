# Parking Lot Control System (PLCS)

Only `std::mutex` and `std::condition_variable` are used.

## Build

    make            # or: cmake -B build && cmake --build build

## Run

    ./plcs [numCars=5] [seconds=0]                         # Activity 2 (unlimited capacity)
    ./plcs 1                                               # Activity 2 step 1: single car
    ./plcs_limited [numCars=6] [capacity=2] [seconds=0]    # Activity 3 (limited capacity)

`seconds = 0` runs until the program is terminated (Ctrl+C).

## Files

- `src/gate.h` - gate thread and the car/gate handshake
- `src/parking_lot.h` - monitor for the free spaces (Activity 3)
- `src/common.h` - logging, stop flag, helpers
- `src/plcs.cpp`, `src/plcs_limited.cpp` - main programs
- `docs/*.puml` - activity diagrams (PlantUML)
- `test_output/` - example runs
