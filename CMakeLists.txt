cmake_minimum_required(VERSION 3.16)
project(plcs CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Threads REQUIRED)

add_executable(plcs src/plcs.cpp)
target_link_libraries(plcs PRIVATE Threads::Threads)

add_executable(plcs_limited src/plcs_limited.cpp)
target_link_libraries(plcs_limited PRIVATE Threads::Threads)
