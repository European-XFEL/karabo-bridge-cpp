cmake_minimum_required(VERSION 2.8.12)
project(client_for_smlt_camera)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -g")

find_package(PkgConfig REQUIRED)

# This is where your zeromq is installed (in the maxwell cluster)
set(ZEROMQ_INSTALL_PREFIX "$ENV{HOME}/share/zeromq")
set(ENV{PKG_CONFIG_PATH} "${ZEROMQ_INSTALL_PREFIX}/lib/pkgconfig/:$ENV{PKG_CONFIG_PATH}")

pkg_check_modules(zmq REQUIRED libzmq>=4.2)

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

set(KBCPP_INSTALL_PREFIX "$ENV{HOME}/share/karabo_bridge_cpp")

include_directories("${KBCPP_INSTALL_PREFIX}/include")

add_executable(smlt_camera_cli client_for_smlt_camera.cpp)
target_link_libraries(smlt_camera_cli ${KBCPP_INSTALL_PREFIX}/lib/libkbcpp.a ${zmq_LIBRARY})