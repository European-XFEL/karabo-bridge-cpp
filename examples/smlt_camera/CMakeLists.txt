cmake_minimum_required(VERSION 2.8.12)
project(client_for_smlt_camera)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -g")

set(INSTALL_PATH "$ENV{HOME}/share")

set(zmq_LIBRARY "${INSTALL_PATH}/zeromq/lib/libzmq.so")

include_directories(${INSTALL_PATH}/zeromq/include)
include_directories(${INSTALL_PATH}/include)

add_executable(smlt_camera_cli client_for_smlt_camera.cpp)
target_link_libraries(smlt_camera_cli ${zmq_LIBRARY})
