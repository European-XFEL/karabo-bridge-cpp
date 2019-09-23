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

project(karabo-bridge LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g")

# =======
# Version
# =======

set(KARABO_BRIDGE_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)

file(STRINGS "${KARABO_BRIDGE_INCLUDE_DIR}/${PROJECT_NAME}/kb_client.hpp" ${PROJECT_NAME}_version_defines
     REGEX "#define KARABO_BRIDGE_VERSION_(MAJOR|MINOR|PATCH)")
foreach(ver ${${PROJECT_NAME}_version_defines})
    if(ver MATCHES "#define KARABO_BRIDGE_VERSION_(MAJOR|MINOR|PATCH) +([^ ]+)$")
        set(KARABO_BRIDGE_VERSION_${CMAKE_MATCH_1} "${CMAKE_MATCH_2}" CACHE INTERNAL "")
    endif()
endforeach()
set(${PROJECT_NAME}_VERSION
    ${KARABO_BRIDGE_VERSION_MAJOR}.${KARABO_BRIDGE_VERSION_MINOR}.${KARABO_BRIDGE_VERSION_PATCH})
message(STATUS "Building karabo-bridge v${${PROJECT_NAME}_VERSION}")

# ============
# Dependencies
# ============

set(cppzmq_REQUIRED_VERSION 4.2.5)
find_package(cppzmq ${cppzmq_REQUIRED_VERSION} REQUIRED)
if(cppzmq_FOUND)
    message(STATUS "Found cppzmq: ${cppzmq_VERSION}, ${cppzmq_INCLUDE_DIR}")
endif()

set(msgpack_REQUIRED_VERSION 3.2.0)
find_package(msgpack ${msgpack_REQUIRED_VERSION} REQUIRED COMPONENT msgpackc-cxx)
if(msgpack_FOUND)
    get_target_property(msgpack_INCLUDE_DIRS msgpackc-cxx INTERFACE_INCLUDE_DIRECTORIES)
    message(STATUS "Found msgpack: ${msgpack_VERSION}, ${msgpack_INCLUDE_DIRS}")
endif()

# =====
# Build
# =====

set(KARABO_BRIDGE_HEADERS ${KARABO_BRIDGE_INCLUDE_DIR}/karabo-bridge/kb_client.hpp)

add_library(karabo-bridge INTERFACE)

target_include_directories(karabo-bridge
    INTERFACE
        $<BUILD_INTERFACE:${KARABO_BRIDGE_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:include>)

target_link_libraries(karabo-bridge INTERFACE cppzmq msgpackc-cxx)

# ==================
# Tests and examples
# ==================

enable_testing()

OPTION(BUILD_TESTS "build unit test" OFF)

OPTION(BUILD_INTEGRATION_TEST "build integration test" OFF)

OPTION(BUILD_EXAMPLES "build examples" OFF)

if (BUILD_TESTS)
    add_subdirectory(tests)
endif()

if (BUILD_INTEGRATION_TEST)
    add_subdirectory(integration_test)
endif()

if (BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# ============
# Installation
# ============

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(FILES ${KARABO_BRIDGE_HEADERS}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})

install(TARGETS karabo-bridge EXPORT ${PROJECT_NAME}-targets)

export(EXPORT ${PROJECT_NAME}-targets
       FILE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake)

configure_package_config_file(
    ${PROJECT_NAME}Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

# compatibility is not karabo-bridge's fault
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    VERSION ${${PROJECT_NAME}_VERSION}
    COMPATIBILITY SameMajorVersion)

install(EXPORT ${PROJECT_NAME}-targets
        FILE ${PROJECT_NAME}Targets.cmake
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")