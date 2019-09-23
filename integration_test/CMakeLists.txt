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

PROJECT(karabo-bridge_integration_test)

if (NOT TARGET karabo-bridge)
    project(karabo-bridge-integration-test)
    find_package(karabo-bridge REQUIRED CONFIG)
endif()

add_executable(pysim_client client_for_pysim.cpp)
target_link_libraries(pysim_client PRIVATE karabo-bridge)
