add_executable(kbcpp-glimpse glimpse.cpp)
target_link_libraries(kbcpp-glimpse kbcpp)

install(TARGETS kbcpp-glimpse DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)