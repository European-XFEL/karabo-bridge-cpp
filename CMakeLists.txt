cmake_minimum_required(VERSION 3.0)
project(karabo-bridge-cpp)

set(CMAKE_CXX_STANDARD 11)

find_package(Threads REQUIRED)

find_package(msgpack 2.1 REQUIRED)

find_package(cppzmq 4.2 REQUIRED)
include_directories(${ZeroMQ_INCLUDE_DIR} ${cppzmq_INCLUDE_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(run src/run_client.cpp include/kb_client.hpp)
target_link_libraries(run ${cppzmq_LIBRARY})

enable_testing()
add_subdirectory(tests)
