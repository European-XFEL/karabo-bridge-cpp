add_executable(kbcpp_glimpse glimpse.cpp)
target_link_libraries(kbcpp_glimpse kbcpp)

install(TARGETS kbcpp_glimpse DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)