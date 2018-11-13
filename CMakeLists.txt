# Maxwell has cmake 2.8.12 and g++ 4.8.5
cmake_minimum_required(VERSION 2.8.12)
project(karabo-bridge-cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -g")

# This is where all external dependencies are installed (in the Maxwell/online cluster)
set(EXTERNAL_INSTALLATION "$ENV{HOME}/share")

include_directories(${EXTERNAL_INSTALLATION}/zeromq/include)
include_directories(${EXTERNAL_INSTALLATION}/msgpack/include)
include_directories(${EXTERNAL_INSTALLATION}/cppzmq/include)

set(zmq_LIBRARY "${EXTERNAL_INSTALLATION}/zeromq/lib/libzmq.so")

add_library(kbcppStatic STATIC kb_client.hpp)
add_library(kbcpp SHARED kb_client.hpp)
target_link_libraries(kbcppStatic ${zmq_LIBRARY})
target_link_libraries(kbcpp ${zmq_LIBRARY})
set_target_properties(kbcppStatic PROPERTIES LINKER_LANGUAGE CXX OUTPUT_NAME kbcpp)
set_target_properties(kbcpp PROPERTIES LINKER_LANGUAGE CXX OUTPUT_NAME kbcpp)

enable_testing()
set(TESTS
    tests/test_version.cpp
    tests/test_multipart_msg.cpp
    tests/test_msgpackobject.cpp
    tests/test_ndarray.cpp
    tests/test_kbdata.cpp)
foreach(filename IN LISTS TESTS)
    string(REPLACE ".cpp" "" targetname ${filename})
    string(REPLACE "tests/" "" targetname ${targetname})
    add_executable(${targetname} ${filename})
    target_link_libraries(${targetname} kbcpp)

    string(TOUPPER ${targetname} testname)
    add_test(${testname} ${targetname})
endforeach()

add_subdirectory(integration_test)
add_subdirectory(tools)

set(KBCPP_INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/karabo_bridge_cpp)
install(TARGETS kbcpp kbcppStatic DESTINATION ${KBCPP_INSTALL_PATH}/lib)
install(FILES kb_client.hpp DESTINATION ${KBCPP_INSTALL_PATH}/include)
