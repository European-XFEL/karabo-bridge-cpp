# Maxwell has cmake 2.8.12 and g++ 4.8.5
cmake_minimum_required(VERSION 2.8)
project(karabo-bridge-cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -Wall")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")

find_package(Threads REQUIRED)

find_package(PkgConfig)
## use pkg-config to get hints for 0mq locations
pkg_check_modules(MY_ZeroMQ QUIET libzmq)

## use the hint from above to find where 'zmq.hpp' is located
find_path(ZeroMQ_INCLUDE_DIR
        NAMES zmq.h
        PATHS ${MY_ZeroMQ_INCLUDE_DIRS}
        )

## use the hint from about to find the location of libzmq
find_library(ZeroMQ_LIBRARY
        NAMES zmq
        PATHS ${MY_ZeroMQ_LIBRARY_DIRS}
        )

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
add_executable(test1 tests/test_version)
add_executable(test2 tests/test_multipart_msg.cpp)
foreach(target test1 test2)
    target_include_directories(${target} PUBLIC ${ZeroMQ_INCLUDE_DIR})
    target_link_libraries(${target} PUBLIC ${ZeroMQ_LIBRARY})
endforeach()

add_test(TEST_VERSION test1)
add_test(TEST_MULTIPART_MSG test2)
