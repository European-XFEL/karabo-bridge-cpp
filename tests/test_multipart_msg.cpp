//
// Author: Jun Zhu, zhujun981661@gmail.com
//
#include "kb_client.hpp"


int main() {
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

    assert(karabo_bridge::parseMultipartMsg(mpmsg, false) ==
           "\"European XFEL\"\n\"is\"\n\"magnificent!\"\n");
}
