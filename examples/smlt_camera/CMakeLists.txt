cmake_minimum_required(VERSION 2.8.12)
project(client_for_smlt_camera)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -g")

set(zmq_LIBRARY "$ENV{HOME}/share/zeromq/lib/libzmq.so")

set(KBCPP_INSTALL_PREFIX "$ENV{HOME}/share/karabo_bridge_cpp")
include_directories("${KBCPP_INSTALL_PREFIX}/include")

add_executable(smlt_camera_cli client_for_smlt_camera.cpp)
target_link_libraries(smlt_camera_cli ${KBCPP_INSTALL_PREFIX}/lib/libkbcpp.a ${zmq_LIBRARY})
