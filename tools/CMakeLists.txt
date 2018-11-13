add_executable(kbcpp-glimpse glimpse.cpp)
target_link_libraries(kbcpp-glimpse kbcpp)

set(KBCPP_INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/karabo_bridge_cpp)
install(TARGETS kbcpp-glimpse DESTINATION ${KBCPP_INSTALL_PATH}/bin)