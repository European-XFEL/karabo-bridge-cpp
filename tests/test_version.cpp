//
// Author: Jun Zhu, zhujun981661@gmail.com
//
#include <cassert>

#include <zmq.hpp>
#include <msgpack.hpp>

static int ZMQ_MAJOR_VERSION = 4;
static int ZMQ_MINOR_VERSION = 2;
static int MSGPACK_MAJOR_VERSION = 2;
static int MSGPACK_MINOR_VERSION = 1;


int main() {
    int major, minor, patch;
    zmq_version(&major, &minor, &patch);
    assert(major >= ZMQ_MAJOR_VERSION);
    assert(minor >= ZMQ_MINOR_VERSION);

    std::string v(MSGPACK_VERSION);
    auto pos = v.find('.');
    assert(std::stoi(v.substr(0, pos)) >= MSGPACK_MAJOR_VERSION);
    v.erase(0, pos + 1);
    pos = v.find('.');
    assert(std::stoi(v.substr(0, pos)) >= MSGPACK_MINOR_VERSION);
}
