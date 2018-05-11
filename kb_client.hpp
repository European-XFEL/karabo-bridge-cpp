/*
    Karabo bridge client.

    Copyright (c) 2018, European X-Ray Free-Electron Laser Facility GmbH
    All rights reserved.

    You should have received a copy of the 3-Clause BSD License along with this
    program. If not, see <https://opensource.org/licenses/BSD-3-Clause>
*/

#ifndef KARABO_BRIDGE_CPP_KB_CLIENT_HPP
#define KARABO_BRIDGE_CPP_KB_CLIENT_HPP

#include "zmq.hpp"

#include <msgpack.hpp>

#include <string>
#include <iostream>
#include <sstream>

namespace karabo_bridge {

/*
 * For deferred unpack
 *
 * typedef enum {
        MSGPACK_OBJECT_NIL                  = 0x00,
        MSGPACK_OBJECT_BOOLEAN              = 0x01,
        MSGPACK_OBJECT_POSITIVE_INTEGER     = 0x02,
        MSGPACK_OBJECT_NEGATIVE_INTEGER     = 0x03,
        MSGPACK_OBJECT_FLOAT32              = 0x0a,
        MSGPACK_OBJECT_FLOAT64              = 0x04,
        MSGPACK_OBJECT_FLOAT                = 0x04,
        MSGPACK_OBJECT_STR                  = 0x05,
        MSGPACK_OBJECT_ARRAY                = 0x06,
        MSGPACK_OBJECT_MAP                  = 0x07,
        MSGPACK_OBJECT_BIN                  = 0x08,
        MSGPACK_OBJECT_EXT                  = 0x09
   } msgpack_object_type;
 */
struct object {
    object() {};
    object(const msgpack::object& value): value_(value) {}

    msgpack::object get() const { return value_; }
    uint16_t type() const { return value_.type; }

private:
    msgpack::object value_;
    MSGPACK_DEFINE(value_);
};

}  // karabo_bridge


namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
namespace adaptor{

/*
 * template specialization for karabo_bridge::object
 */

template<>
struct as<karabo_bridge::object> {
    karabo_bridge::object operator()(msgpack::object const& o) const {
        return karabo_bridge::object(o.as<msgpack::object>());
    }
};

} // adaptor
} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // msgpack


namespace karabo_bridge {
/*
 * visitor used to unfold the hierarchy of a unknown dictionary-like
 * data structure
 */
struct karabo_visitor {
    std::string& m_s;
    bool m_ref;

    explicit karabo_visitor(std::string& s):m_s(s), m_ref(false) {}

    bool visit_nil() {
        m_s += "null";
        return true;
    }
    bool visit_boolean(bool v) {
        if (v) m_s += "true";
        else m_s += "false";
        return true;
    }
    bool visit_positive_integer(uint64_t v) {
        std::stringstream ss;
        ss << v;
        m_s += ss.str();
        return true;
    }
    bool visit_negative_integer(int64_t v) {
        std::stringstream ss;
        ss << v;
        m_s += ss.str();
        return true;
    }
    bool visit_float32(float v) {
        std::stringstream ss;
        ss << v;
        m_s += ss.str();
        return true;
    }
    bool visit_float64(double v) {
        std::stringstream ss;
        ss << v;
        m_s += ss.str();
        return true;
    }
    bool visit_str(const char* v, uint32_t size) {
        // I omit escape process.
        m_s += '"' + std::string(v, size) + '"';
        return true;
    }
    bool visit_bin(const char* /*v*/, uint32_t /*size*/) {
        return true;
    }
    bool visit_ext(const char* /*v*/, uint32_t /*size*/) {
        return true;
    }
    bool start_array_item() {
        return true;
    }
    bool start_array(uint32_t /*num_elements*/) {
        m_s += "[";
        return true;
    }
    bool end_array_item() {
        m_s += ",";
        return true;
    }
    bool end_array() {
        m_s.erase(m_s.size() - 1, 1); // remove the last ','
        m_s += "]";
        return true;
    }
    bool start_map(uint32_t /*num_kv_pairs*/) {
        m_s += "{";
        return true;
    }
    bool start_map_key() {
        return true;
    }
    bool end_map_key() {
        m_s += ":";
        return true;
    }
    bool start_map_value() {
        return true;
    }
    bool end_map_value() {
        m_s += ",";
        return true;
    }
    bool end_map() {
        m_s.erase(m_s.size() - 1, 1); // remove the last ','
        m_s += "}";
        return true;
    }
    void parse_error(size_t /*parsed_offset*/, size_t /*error_offset*/) {
        std::cerr << "parse error"<<std::endl;
    }
    void insufficient_bytes(size_t /*parsed_offset*/, size_t /*error_offset*/) {
        std::cout << "insufficient bytes"<<std::endl;
    }

    // These two functions are required by parser.
    void set_referenced(bool ref) { m_ref = ref; }
    bool referenced() const { return m_ref; }
};


inline void msg2str(zmq::message_t& msg, std::string& str) {
    str = std::string(static_cast<const char *>(msg.data()), msg.size());
}

/*
 * data presented to the user
 */
struct kb_data {
    std::string source;
    std::map<std::string, object> data;

    object& operator[](std::string key) {
        return data[key];
    }
};

/*
 * Helper function.
 */
void printKBData(kb_data dt) {
    for (auto& v: dt.data) {
        std::cout << v.first << ": " << v.second.type();
        if (v.second.type() == 2) std::cout << ", " << v.second.get();
        std::cout << "\n";
    }
}

/*
 * Karabo-bridge Client class.
 */
class Client {
    zmq::context_t ctx_;
    zmq::socket_t socket_;

    /*
     * Send a request to server.
     *
     * :param req: request. Default to "next".
     */
    void send_request(const char* req="next") {
        zmq::message_t request(4);
        memcpy(request.data(), req, request.size());
        socket_.send(request);
    }

    /*
     * Receive a multipart message from the server.
     */
    zmq::message_t receive_multipart_msg() {
        zmq::message_t msg;
        int64_t more;  // multipart checker
        while (true) {
            socket_.recv(&msg);
            std::size_t more_size = sizeof(int64_t);
            socket_.getsockopt(ZMQ_RCVMORE, &more, &more_size);
            if (more == 0) break;
        }

        return msg;
    }

public:
    Client(): ctx_(1), socket_(ctx_, ZMQ_REQ) {}

    void connect(const std::string& endpoint) {
        std::cout << "Connecting to server: " << endpoint << std::endl;
        socket_.connect(endpoint.c_str());
    }

    /*
     * Request and return the next data from the server.
     */
    kb_data next() {
        kb_data kbdt;

        send_request();
        auto msg = receive_multipart_msg();

        using MsgObjectMap = std::map<std::string, msgpack::object>;
        using KBObjectMap = std::map<std::string, object>;

        msgpack::object_handle result;
        msgpack::unpack(result, static_cast<const char*>(msg.data()), msg.size());
        auto root_unpacked = result.get().as<MsgObjectMap>();
        assert(root_unpacked.size() == 1);

        for (auto &v : root_unpacked) {
            kbdt.source = v.first;

            auto data_unpacked = v.second.as<KBObjectMap>();
            for (auto &v : data_unpacked) {
                kbdt.data.insert(std::make_pair(v.first, v.second));
            }
        }

        return kbdt;
    }

    /*
     * Dump next data to JSON format
     */
    void dump_next() {
        send_request();
        auto msg = receive_multipart_msg();

        // For now, we put the unpacked data into a string
        std::string data_str;
        karabo_visitor visitor(data_str);
        bool ret = msgpack::v2::parse(static_cast<const char*>(msg.data()), msg.size(), visitor);
        assert(ret);
        std::cout << "Unpacked result: " << data_str << "\n";
    }
};

} // karabo_bridge

#endif //KARABO_BRIDGE_CPP_KB_CLIENT_HPP
