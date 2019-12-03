//
// Author: Jun Zhu, zhujun981661@gmail.com
//
#include <iostream>
#include <future>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "karabo-bridge/kb_client.hpp"


namespace karabo_bridge {

using ::testing::ElementsAre;
using ::testing::ElementsAreArray;


TEST(TestClient, TestTimeout) {
    // test client with short timeout
    int timeout = 100; // in millisecond
    Client client(0.001 * timeout);
    client.connect("tcp://localhost:12345");

    auto future = std::async(std::launch::async, [&client]() {
        // test the internal recv_ready_ flag
        client.next();
        client.next();
    });
    EXPECT_TRUE(future.wait_for(std::chrono::milliseconds(3*timeout)) == std::future_status::ready);

    //  test client without timeout
    auto client_inf = new Client;
    client_inf->connect("tcp://localhost:12346");

    auto future_inf = std::async(std::launch::async, [&client_inf]() {
        client_inf->next();
    });
    EXPECT_TRUE(future_inf.wait_for(std::chrono::milliseconds(2*timeout)) == std::future_status::timeout);
    delete client_inf; // close the blocking socket
}

} // karabo_bridge