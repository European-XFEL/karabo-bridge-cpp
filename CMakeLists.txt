# Maxwell has cmake 2.8.12 and g++ 4.8.5
cmake_minimum_required(VERSION 2.8.12)
project(karabo-bridge-cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -g")

find_package(PkgConfig REQUIRED)

# This is where your zeromq is installed (in the maxwell cluster)
set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/share/zeromq")
set(ENV{PKG_CONFIG_PATH} "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig/:$ENV{PKG_CONFIG_PATH}")

pkg_check_modules(zmq REQUIRED libzmq>=4.2)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/msgpack/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/cppzmq)
include_directories(${zmq_INCLUDE_DIRS})

find_library(zmq_LIBRARY
             NAMES zmq
             PATHS ${zmq_LIBRARY_DIRS})

if (${zmq_LIBRARY_DIRS} STREQUAL "/usr/local/lib")
else()
    # One must specify the library file, otherwise CMAKE may specify the 
    # default (system) library which has a different version.
    set(zmq_LIBRARY "${zmq_LIBRARY_DIRS}/libzmq.so")
endif()

add_executable(run1 src/client_for_pysim.cpp include/kb_client.hpp)
add_executable(run2 src/client_for_smlt_camera.cpp)
add_executable(glimpse src/glimpse.cpp)
foreach(target run1 run2 glimpse)
    target_link_libraries(${target} ${zmq_LIBRARY})
endforeach()

enable_testing()
add_executable(test1 tests/test_version)
add_executable(test2 tests/test_multipart_msg.cpp)
add_executable(test3 tests/test_object.cpp)
add_executable(test4 tests/test_array.cpp)
foreach(target test1 test2 test3 test4)
    target_link_libraries(${target} ${zmq_LIBRARY})
endforeach()

add_test(TEST_VERSION test1)
add_test(TEST_MULTIPART_MSG test2)
add_test(TEST_OBJECT test3)
add_test(TEST_ARRAY test4)
