set(KBCPP_TESTS
    test_version.cpp
    test_multipart_msg.cpp
    test_msgpackobject.cpp
    test_ndarray.cpp
    test_kbdata.cpp)

foreach(filename IN LISTS KBCPP_TESTS)
    string(REPLACE ".cpp" "" targetname ${filename})
    string(REPLACE "tests/" "" targetname ${targetname})
    add_executable(${targetname} ${filename})
    target_link_libraries(${targetname} kbcpp)

    string(TOUPPER ${targetname} testname)
    add_test(${testname} ${targetname})
endforeach()
