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
    project(karabo-bridge_glimps)
    find_package(karabo-bridge REQUIRED CONFIG)
endif()

get_target_property(karabo-bridge_LINK_LIBRARY karabo-bridge INTERFACE_LINK_LIBRARIES)

add_executable(glimpse glimpse.cpp)
target_link_libraries(glimpse PRIVATE karabo-bridge karabo-bridge ${karabo-bridge_LINK_LIBRARIES})
