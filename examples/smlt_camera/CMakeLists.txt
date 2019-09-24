##############################################################################
# Copyright (c) 2018, European X-Ray Free-Electron Laser Facility GmbH
# All rights reserved.
#
# You should have received a copy of the 3-Clause BSD License along with this
# program. If not, see <https://opensource.org/licenses/BSD-3-Clause>
#
# Author: Jun Zhu, zhujun981661@gmail.com
##############################################################################

cmake_minimum_required(VERSION 3.1)

if (NOT TARGET karabo-bridge)
    project(karabo-bridge_simulated_camera)
    find_package(karabo-bridge REQUIRED CONFIG)
endif()

add_executable(camera_cli camera_client.cpp)
target_link_libraries(camera_cli PRIVATE karabo-bridge)