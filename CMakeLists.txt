# Maxwell has cmake 2.8.12 and g++ 4.8.5
cmake_minimum_required(VERSION 2.8.12)
project(karabo-bridge-cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -g")

# We assume all the dependencies have been installed in the same folder
include_directories(${CMAKE_INSTALL_PREFIX}/zeromq/include)
include_directories(${CMAKE_INSTALL_PREFIX}/include)

set(zmq_LIBRARY "${CMAKE_INSTALL_PREFIX}/zeromq/lib/libzmq.so")

add_library(kbcpp SHARED kb_client.hpp)
target_link_libraries(kbcpp ${zmq_LIBRARY})
set_target_properties(kbcpp PROPERTIES LINKER_LANGUAGE CXX)

enable_testing()

OPTION(BUILD_TESTS "build unit test" OFF)
OPTION(BUILD_INTEGRATION_TEST "build integration test" OFF)

if (BUILD_TESTS)
    add_subdirectory(tests)
endif()

if (BUILD_INTEGRATION_TEST)
    add_subdirectory(integration_test)
endif()

add_subdirectory(tools)

install(FILES kb_client.hpp DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
