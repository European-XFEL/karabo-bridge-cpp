# Maxwell has cmake 2.8.12 and g++ 4.8.5
cmake_minimum_required(VERSION 2.8.12)
project(karabo-bridge-cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -g")

find_package(PkgConfig REQUIRED)

set(EXTERNAL_INSTALLATION "$ENV{HOME}/share")
# This is where your zeromq is installed (in the maxwell cluster)
set(ZEROMQ_INSTALL_PREFIX "${EXTERNAL_INSTALLATION}/zeromq")
set(ENV{PKG_CONFIG_PATH} "${ZEROMQ_INSTALL_PREFIX}/lib/pkgconfig/:$ENV{PKG_CONFIG_PATH}")

pkg_check_modules(zmq REQUIRED libzmq>=4.2)

include_directories(${EXTERNAL_INSTALLATION}/msgpack/include)
include_directories(${EXTERNAL_INSTALLATION}/cppzmq/include)
include_directories(${zmq_INCLUDE_DIRS})

#set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
find_library(zmq_LIBRARY
             NAMES zmq
             PATHS ${zmq_LIBRARY_DIRS})

if (${zmq_LIBRARY_DIRS} STREQUAL "/usr/local/lib")
else()
    # One must specify the library file, otherwise CMAKE may specify the
    # default (system) library which has a different version.
    set(zmq_LIBRARY "${zmq_LIBRARY_DIRS}/libzmq.so")
endif()

add_library(kbcpp STATIC kb_client.hpp)
target_link_libraries(kbcpp ${zmq_LIBRARY})
set_target_properties(kbcpp PROPERTIES LINKER_LANGUAGE CXX)

enable_testing()
add_executable(test1 tests/test_version)
add_executable(test2 tests/test_multipart_msg.cpp)
add_executable(test3 tests/test_msgpackobject.cpp)
add_executable(test4 tests/test_ndarray.cpp)
add_executable(test5 tests/test_kbdata.cpp)
foreach(target test1 test2 test3 test4 test5)
    target_link_libraries(${target} kbcpp)
endforeach()

add_test(TEST_VERSION test1)
add_test(TEST_MULTIPART_MSG test2)
add_test(TEST_MSGPACK_OBJECT test3)
add_test(TEST_NDARRAY test4)
add_test(TEST_KBDATA test5)

add_subdirectory(integration_test)
add_subdirectory(tools)

install(TARGETS kbcpp DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(FILES kb_client.hpp DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
