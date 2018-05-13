cmake_minimum_required(VERSION 3.0)
project(karabo-bridge-cpp)

set(CMAKE_CXX_STANDARD 11)

find_package(Threads REQUIRED)
find_package(ZeroMQ 4.2 REQUIRED)
find_package(msgpack 2.1 REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/include)

enable_testing()
add_subdirectory(tests)