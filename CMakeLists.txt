cmake_minimum_required(VERSION 3.0)
project(karabo-bridge-cpp)

set(CMAKE_CXX_STANDARD 11)

find_package(Threads REQUIRED)

find_package(msgpack 2.1 REQUIRED)

find_package(cppzmq 4.2 REQUIRED)
include_directories(${ZeroMQ_INCLUDE_DIR} ${cppzmq_INCLUDE_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(run1 src/client_for_pysim.cpp include/kb_client.hpp)
add_executable(run2 src/client_for_smlt_camera.cpp include/kb_client.hpp)
foreach(target run1 run2)
    target_link_libraries(${target} ${cppzmq_LIBRARY})
endforeach()

enable_testing()
add_subdirectory(tests)
