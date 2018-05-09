find_package(GTest REQUIRED)

include_directories(${GTest_INCLUDE_DIRS})

add_executable(Test test.cpp)

target_link_libraries(Test
        libzmq
        gmock
        gtest
        pthread
)



add_test (NAME UnitTest COMMAND Test)