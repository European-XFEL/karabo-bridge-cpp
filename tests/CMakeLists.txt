cmake_minimum_required(VERSION 3.1)

PROJECT(karabo-bridge_unittest)

set(unittests
    test_multipart_msg.cpp
    test_msgpackobject.cpp
    test_ndarray.cpp
    test_kbdata.cpp)

foreach(filename IN LISTS unittests)
    string(REPLACE ".cpp" "" targetname ${filename})
    string(REPLACE "tests/" "" targetname ${targetname})
    add_executable(${targetname} ${filename})
    target_link_libraries(${targetname} PRIVATE karabo-bridge ${karabo-bridge_LINK_LIBRARIES})

    string(TOUPPER ${targetname} testname)
    add_test(${testname} ${targetname})
endforeach()
