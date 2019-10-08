cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

PROJECT(karabo-bridge-unittest)

include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.8.1)
FetchContent_GetProperties(googletest)
if(NOT googletest_POPULATED)
    FetchContent_Populate(googletest)
    add_subdirectory(
        ${googletest_SOURCE_DIR}
        ${googletest_BINARY_DIR}
        EXCLUDE_FROM_ALL
    )
endif()

find_package(Threads REQUIRED)

add_executable(test_karabo-bridge test_kbclient.cpp)
target_link_libraries(test_karabo-bridge
    PRIVATE
        karabo-bridge
    PRIVATE
        gmock
        gmock_main
        pthread)

add_custom_target(
    kbtest
    COMMAND test_karabo-bridge
    DEPENDS test_karabo-bridge)