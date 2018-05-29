cmake_minimum_required(VERSION 2.8) # Maxwell has cmake 2.8.12 and g++ 4.8.5

project(karabo-bridge-cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -Wall")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")

find_package(Threads REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/msgpack)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/cppzmq)

add_executable(run1 src/client_for_pysim.cpp include/kb_client.hpp)
add_executable(run2 src/client_for_smlt_camera.cpp include/kb_client.hpp)
foreach(target run1 run2)
    target_include_directories(${target} PUBLIC ${ZeroMQ_INCLUDE_DIR})
    target_link_libraries(${target} PUBLIC ${ZeroMQ_LIBRARY})
endforeach()

enable_testing()
add_subdirectory(tests)
