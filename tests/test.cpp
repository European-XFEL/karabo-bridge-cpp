#include "zmq.hpp"
#include "kb_client.hpp"

#include <gtest/gtest.h>
#include <gmock/gmock.h>


using ::testing::UnorderedElementsAre;
using ::testing::ElementsAre;


TEST(TestZMQ, VERSION) {
    int major, minor, patch;
    zmq_version (&major, &minor, &patch);

    ASSERT_GE(major, 4);
    ASSERT_GE(minor, 2);
}

TEST(TestMsgpack, VERSION) {
    int major, minor, patch;
    std::cout << MSGPACK_VERSION << std::endl;
}

TEST(Server4Test, GeneralTest) {
    auto client = karabo_bridge::Client();
    client.connect("tcp://localhost:1234");
    client.next();
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}