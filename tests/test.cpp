#include "kb_client.hpp"

#include <thread>
#include <vector>
#include <algorithm>

#include <gtest/gtest.h>
#include <gmock/gmock.h>


using ::testing::UnorderedElementsAre;
using ::testing::ElementsAre;

static int ZMQ_MAJOR_VERSION = 4;
static int ZMQ_MINOR_VERSION = 1;
static int MSGPACK_MAJOR_VERSION = 2;
static int MSGPACK_MINOR_VERSION = 1;


TEST(ZMQTest, Version) {
    int major, minor, patch;
    zmq_version (&major, &minor, &patch);
    ASSERT_GE(major, ZMQ_MAJOR_VERSION);
    ASSERT_GE(minor, ZMQ_MINOR_VERSION);
}

TEST(MsgpackTest, Version) {
    std::string v(MSGPACK_VERSION);
    auto pos = v.find('.');
    EXPECT_EQ(std::stoi(v.substr(0, pos)), MSGPACK_MAJOR_VERSION);
    v.erase(0, pos + 1);
    pos = v.find('.');
    EXPECT_EQ(std::stoi(v.substr(0, pos)), MSGPACK_MINOR_VERSION);
}

TEST(parseMultipartMsgTest, multipartMessage) {
    karabo_bridge::MultipartMsg mpmsg;

    std::stringstream ss;
    msgpack::pack(ss, "European XFEL");
    mpmsg.emplace_back(zmq::message_t(ss.str().data(), ss.str().size()));

    ss.str("");
    msgpack::pack(ss, "is");
    mpmsg.emplace_back(zmq::message_t(ss.str().data(), ss.str().size()));

    ss.str("");
    msgpack::pack(ss, "magnificent!");
    mpmsg.emplace_back(zmq::message_t(ss.str().data(), ss.str().size()));

    EXPECT_EQ(karabo_bridge::parseMultipartMsg(mpmsg, false),
              "\"European XFEL\"\n\"is\"\n\"magnificent!\"\n");
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
