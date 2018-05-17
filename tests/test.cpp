#include "kb_client.hpp"

#include <thread>
#include <vector>
#include <algorithm>

#include <gtest/gtest.h>
#include <gmock/gmock.h>


using ::testing::UnorderedElementsAre;
using ::testing::ElementsAre;

static int MSGPACK_MAJOR_VERSION = 2;
static int MSGPACK_MINOR_VERSION = 1;


TEST(ZMQTest, Version) {
    int major, minor, patch;
    zmq_version (&major, &minor, &patch);
    ASSERT_GE(major, 4);
    ASSERT_GE(minor, 2);
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
    karabo_bridge::multipart_msg mpmsg;

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

class TestClientWithPythonSimulator: public testing::Test {
protected:
    karabo_bridge::Client client_;

    virtual void SetUp() override {
        client_.connect("tcp://localhost:1234");
    }
};

TEST_F(TestClientWithPythonSimulator, showNext) {
    client_.showNext();
}

TEST_F(TestClientWithPythonSimulator, Next) {
    auto result = client_.next();

    EXPECT_EQ(result.metadata["source"].as<std::string>(), "SPB_DET_AGIPD1M-1/DET/detector");
    std::cout << "timestamp.tid: " << result.metadata["timestamp.tid"].as<uint64_t>() << "\n";
    std::cout << "timestamp.sec: " << result.metadata["timestamp.sec"].as<std::string>() << "\n";
    std::cout << "timestamp.frac: " << result.metadata["timestamp.frac"].as<std::string>() << "\n";

    EXPECT_EQ(result["header.pulseCount"].as<uint64_t>(), 32);
    EXPECT_EQ(result["trailer.status"].as<uint64_t>(), 0);
    EXPECT_EQ(result["header.majorTrainFormatVersion"].as<uint64_t>(), 2);
    EXPECT_EQ(result["header.minorTrainFormatVersion"].as<uint64_t>(), 1);
    EXPECT_THROW(result["header"], std::out_of_range);

    auto train_id = result["image.trainId"].asArray<uint64_t, 32>();
    EXPECT_EQ(std::adjacent_find(train_id.begin(), train_id.end(), std::not_equal_to<int>()),
              train_id.end());
    EXPECT_GT(train_id[0], 10000000000);
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
