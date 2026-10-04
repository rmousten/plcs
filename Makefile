CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -pthread -O2

all: plcs plcs_limited

plcs: src/plcs.cpp src/gate.h src/common.h
	$(CXX) $(CXXFLAGS) $< -o $@

plcs_limited: src/plcs_limited.cpp src/gate.h src/parking_lot.h src/common.h
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm -f plcs plcs_limited

.PHONY: all clean
