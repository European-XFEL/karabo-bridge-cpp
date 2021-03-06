/*
    Karabo bridge client.

    Copyright (c) 2018, European X-Ray Free-Electron Laser Facility GmbH
    All rights reserved.

    You should have received a copy of the 3-Clause BSD License along with this
    program. If not, see <https://opensource.org/licenses/BSD-3-Clause>

    Author: Jun Zhu, zhujun981661@gmail.com
*/

#ifndef KARABO_BRIDGE_KB_CLIENT_HPP
#define KARABO_BRIDGE_KB_CLIENT_HPP

#include <zmq.hpp>
#include <msgpack.hpp>

#include <string>
#include <stack>
#include <array>
#include <deque>
#include <iostream>
#include <sstream>
#include <fstream>
#include <exception>
#include <limits>
#include <type_traits>


#ifdef __GNUC__
#define DEPRECATED __attribute__ ((deprecated))
#elif defined(_MSC_VER)
#define DEPRECATED __declspec(deprecated)
#else
#define DEPRECATED
#pragma message("DEPRECATED is not defined for this compiler")
#endif

#define KARABO_BRIDGE_VERSION_MAJOR 0
#define KARABO_BRIDGE_VERSION_MINOR 2
#define KARABO_BRIDGE_VERSION_PATCH 0


namespace karabo_bridge {

using MultipartMsg = std::deque<zmq::message_t>;

// Define exceptions to ease debugging

class CastError : public std::exception {
    std::string msg_;
public:
    explicit CastError(const std::string& msg) : msg_(msg) {}
    virtual const char* what() const throw() {
        return msg_.c_str() ;
    }
};

class TypeMismatchErrorNDArray : public CastError {
public:
    explicit TypeMismatchErrorNDArray(const std::string& msg)
        : CastError(msg)
    {}
};

class CastErrorMsgpackObject : public CastError {
public:
    explicit CastErrorMsgpackObject(const std::string& msg) : CastError(msg)
    {}
};

class CastErrorNDArray : public CastError {
public:
    explicit CastErrorNDArray(const std::string& msg) : CastError(msg)
    {}
};

class ZmqTimeoutError : public std::runtime_error {
public:
  ZmqTimeoutError() : std::runtime_error("") {}
};

/*
 * Abstract class for MsgpackObject and NDArray.
 */
class Object {

protected:
    // size of the flattened array, 0 for scalar data and NIL
    std::size_t size_;
    // data type, if container, it refers to the data type in the container
    std::string dtype_;

public:
    Object() : size_(0) {};
    virtual ~Object() = default;

    virtual std::size_t size() const = 0;
    virtual std::string dtype() const = 0;
    // empty vector for scalar data
    virtual std::vector<std::size_t> shape() const = 0;
    // empty string for scalar data
    virtual std::string containerType() const = 0;
};

/*
 * A container that holds a msgpack::object for deferred unpack.
 */
class MsgpackObject : public Object {

    msgpack::object value_; // msgpack::object has a shallow copy constructor

public:
    MsgpackObject() = default;  // must be default constructable

    explicit MsgpackObject(const msgpack::object& value): value_(value) {
        if (value.type == msgpack::type::object_type::ARRAY
                || value.type == msgpack::type::object_type::MAP
                || value.type == msgpack::type::object_type::BIN)
            size_ = value.via.array.size;

        if (value.type == msgpack::type::object_type::ARRAY)
            if (value.via.array.ptr)
                dtype_ = getTypeString(value.via.array.ptr[0].type);
            else
                dtype_ = "unknown";
        else if (value.type == msgpack::type::object_type::BIN)
            dtype_ = "char";
        else if (value.type == msgpack::type::object_type::MAP
                || value.type == msgpack::type::object_type::EXT)
            dtype_ = "undefined";
        else dtype_ = getTypeString(value.type);
    }

    ~MsgpackObject() override = default;

    MsgpackObject(const MsgpackObject&) = default;
    MsgpackObject& operator=(const MsgpackObject&) = default;

    MsgpackObject(MsgpackObject&&) = default;
    MsgpackObject& operator=(MsgpackObject&&) = default;

    /*
     * Cast the held msgpack::object to a given type.
     *
     * Exceptions:
     * CastErrorMsgpackObject: if cast fails
     */
    template<typename T>
    T as() const {
        try {
            return value_.as<T>();
        } catch(std::bad_cast& e) {
            std::string error_msg;
            if (size_)
                error_msg = ("The expected type is a(n) " + containerType() + " of " + dtype());
            else
                error_msg = ("The expected type is " + dtype());

            throw CastErrorMsgpackObject(error_msg);
        }
    }

    std::string dtype() const override { return dtype_; }

    std::size_t size() const override { return size_; }

    std::vector<std::size_t> shape() const override {
        if (size_) return std::vector<std::size_t>({size_});
        return std::vector<std::size_t>();
    }

    std::string containerType() const override {
        if (!size_) return ""; // scalar and NIL
        if (value_.type == msgpack::type::object_type::ARRAY
                || value_.type == msgpack::type::object_type::BIN)
            return "array-like";
        if (value_.type == msgpack::type::object_type::MAP) return "map";
        return getTypeString(value_.type);
    }

private:
    // map msgpack object types to strings
    static std::string getTypeString(msgpack::type::object_type type) {
        const std::map<msgpack::type::object_type, std::string> map {
            {msgpack::type::object_type::NIL, "MSGPACK_OBJECT_NIL"},
            {msgpack::type::object_type::BOOLEAN, "bool"},
            {msgpack::type::object_type::POSITIVE_INTEGER, "uint64_t"},
            {msgpack::type::object_type::NEGATIVE_INTEGER, "int64_t"},
            {msgpack::type::object_type::FLOAT32, "float"},
            {msgpack::type::object_type::FLOAT64, "double"},
            {msgpack::type::object_type::STR, "string"},
            {msgpack::type::object_type::ARRAY, "MSGPACK_OBJECT_ARRAY"},
            {msgpack::type::object_type::MAP, "MSGPACK_OBJECT_MAP"},
            {msgpack::type::object_type::BIN, "MSGPACK_OBJECT_BIN"},
            {msgpack::type::object_type::EXT, "MSGPACK_OBJECT_EXT"}
        };

        return map.at(type);
    }
};

using ObjectMap = std::map<std::string, MsgpackObject>;
using ObjectPair = std::pair<std::string, MsgpackObject>;

namespace detail {

template<typename Container, typename ElementType>
struct as_imp {
    Container operator()(void* ptr_, std::size_t size) {
        auto ptr = reinterpret_cast<const ElementType*>(ptr_);
        return Container(ptr, ptr + size);
    }
};

// partial specialization for std::array
template<typename ElementType, std::size_t N>
struct as_imp<std::array<ElementType, N>, ElementType> {
    std::array<ElementType, N> operator()(void*ptr_, std::size_t size) {
        if (size != N)
            throw CastErrorNDArray("The input size " + std::to_string(N) +
                " is different from the expected size " + std::to_string(size));
        auto ptr = reinterpret_cast<const ElementType*>(ptr_);
        std::array<ElementType, N> arr;
        memcpy(arr.data(), ptr, size * sizeof(ElementType));
        return arr;
    }
};

}  // detail

/*
 * A container held a pointer to the data chunk and other useful information.
 */
class NDArray : public Object {

    void* ptr_; // pointer to the data chunk
    std::vector<std::size_t> shape_; // shape of the array

public:
    NDArray() = default;

    // shape and dtype should be moved into the constructor
    NDArray(void* ptr, const std::vector<std::size_t>& shape, const std::string& dtype):
            ptr_(ptr), shape_(shape) {
        std::size_t size = 1;
        // Overflow is not expected since otherwise zmq::message_t
        // cannot hold the data.
        for (auto& v : shape) size *= v;
        size_ = size;
        dtype_ = dtype;
    }

    ~NDArray() override = default;

    NDArray(const NDArray&) = default;
    NDArray& operator=(const NDArray&) = default;

    NDArray(NDArray&&) = default;
    NDArray& operator=(NDArray&&) = default;

    std::size_t size() const override { return size_; }

    /*
     * Copy the data into a vector.
     *
     * Exceptions:
     * TypeMismatchErrorNDArray: if type mismatches
     * CastErrorNDArray: if cast fails
     */
    template<typename Container,
             typename = typename std::enable_if<!std::is_integral<Container>::value>::type>
    Container as() const {
        typedef typename Container::value_type ElementType;
        if (!validateType<ElementType>(dtype_))
            throw TypeMismatchErrorNDArray(
                "The expected type is a(n) " + containerType() + " of " + dtype());
        detail::as_imp<Container, ElementType> as_imp_instance;
        return as_imp_instance(ptr_, size());
    }

    std::vector<std::size_t> shape() const override { return shape_; }

    std::string dtype() const override { return dtype_; }

    std::string containerType() const override { return "array-like"; }

    /*
     * Return a casted pointer to the held array data.
     *
     * Exceptions:
     * TypeMismatchErrorNDArray: if type mismatches
     */
    template<typename T>
    T* data() const {
        if (!validateType<T>(dtype_))
            throw TypeMismatchErrorNDArray("The expected pointer type is " + dtype());
        return reinterpret_cast<T*>(ptr_);
    }

    // Return a void pointer to the held array data.
    void* data() const { return ptr_; }

private:
    /*
     * Use to check data type before casting an NDArray object.
     *
     * Implicit type conversion is not allowed.
     */
    template <typename T>
    static bool validateType(const std::string& type_string) {
        if (type_string == "uint64_t" && std::is_same<T, uint64_t>::value) return true;
        if (type_string == "uint32_t" && std::is_same<T, uint32_t>::value) return true;
        if (type_string == "uint16_t" && std::is_same<T, uint16_t>::value) return true;
        if (type_string == "uint8_t" && std::is_same<T, uint8_t>::value) return true;
        if (type_string == "int64_t" && std::is_same<T, int64_t>::value) return true;
        if (type_string == "int32_t" && std::is_same<T, int32_t>::value) return true;
        if (type_string == "int16_t" && std::is_same<T, int16_t>::value) return true;
        if (type_string == "int8_t" && std::is_same<T, int8_t>::value) return true;
        if (type_string == "float" && std::is_same<T, float>::value) return true;
        if (type_string == "double" && std::is_same<T, double>::value) return true;
        return (type_string == "bool" && std::is_same<T, bool>::value);
    }
};

} // karabo_bridge


namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
namespace adaptor{

/*
 * template specialization for karabo_bridge::object
 */
template<>
struct as<karabo_bridge::MsgpackObject> {
    karabo_bridge::MsgpackObject operator()(msgpack::object const& o) const {
        return karabo_bridge::MsgpackObject(o.as<msgpack::object>());
    }
};

} // adaptor
} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // msgpack


namespace karabo_bridge {

/*
 * Data structure presented to the user.
 *
 * - The data member "metadata" holds a map of meta data;
 * - The data member "array" holds a map of array data, which is usually a
 *   big chunk of data;
 * - The data member "data_" holds a map of normal data, which can be either
 *   scalar data or small arrays.
 */
struct kb_data {
    kb_data() = default;

    ~kb_data() = default;

    kb_data(const kb_data&) = delete;
    kb_data& operator=(const kb_data&) = delete;

    kb_data(kb_data&&) = default;
    kb_data& operator=(kb_data&&) = default;

    using iterator = ObjectMap::iterator;
    using const_iterator = ObjectMap::const_iterator;

    ObjectMap metadata;
    std::map<std::string, NDArray> array;

    MsgpackObject& operator[](const std::string& key) { return data_.at(key); }

    iterator begin() noexcept { return data_.begin(); }
    iterator end() noexcept { return data_.end(); }
    const_iterator begin() const noexcept { return data_.begin(); }
    const_iterator end() const noexcept { return data_.end(); }
    const_iterator cbegin() const noexcept { return data_.cbegin(); }
    const_iterator cend() const noexcept { return data_.cend(); }

    template<typename T>
    std::pair<iterator, bool> insert(T&& value) {
        return data_.insert(std::forward<T>(value));
    }

    std::size_t bytesReceived() const {
        std::size_t size_ = 0;
        for (auto& m : mpmsg_) size_ += m.size();
        return size_;
    }

    void appendMsg(zmq::message_t&& msg) {
        mpmsg_.push_back(std::move(msg));
    }

    void appendHandle(msgpack::object_handle&& oh) {
        handles_.push_back(std::move(oh));
    }

    void swap(kb_data& other) {
        metadata.swap(other.metadata);
        array.swap(other.array);
        data_.swap(other.data_);
        mpmsg_.swap(other.mpmsg_);
        handles_.swap(other.handles_);
    }

private:
    ObjectMap data_;
    std::vector<zmq::message_t> mpmsg_; // maintain the lifetime of data
    std::vector<msgpack::object_handle> handles_; // maintain the lifetime of data
};

/*
 * Convert a vector to a formatted string
 */
template <typename T>
std::string vectorToString(const std::vector<T>& vec) {
    if (vec.empty()) return "";

    std::stringstream ss;
    ss << "[";
    for (std::size_t i=0; i<vec.size(); ++i) {
        ss << vec[i];
        if (i != vec.size() - 1) ss << ", ";
    }
    ss << "]";
    return ss.str();
}

/*
 * Convert the python type to the corresponding C++ type
 */
inline void toCppTypeString(std::string& dtype) {
    if (dtype.find("int") != std::string::npos)
        dtype.append("_t");
    else if (dtype == "float32")
        dtype = "float";
    else if (dtype == "float64")
        dtype = "double";
}


/*
 * Karabo-bridge Client class.
 */
class Client {
    zmq::context_t ctx_;
    zmq::socket_t socket_;

    // Set to true if the client has sent request to the server to ask
    // for data.
    bool recv_ready_ = false;

    /*
     * Send a "next" request to server.
     */
    void sendRequest() {
        zmq::message_t request(4);
        memcpy(request.data(), "next", request.size());
        socket_.send(request);
    }

    /*
     * Receive a multipart message from the server.
     */
    MultipartMsg receiveMultipartMsg() {
        int64_t more;  // multipart checker
        MultipartMsg mpmsg;
        while (true) {
            zmq::message_t msg;
            auto flag = socket_.recv(&msg);
            if (!flag) throw ZmqTimeoutError();

            mpmsg.emplace_back(std::move(msg));
            std::size_t more_size = sizeof(int64_t);
            socket_.getsockopt(ZMQ_RCVMORE, &more, &more_size);
            if (more == 0) break;
        }
        return mpmsg;
    }

    /*
     * Parse a single message packed by msgpack using "visitor".
     */
    static std::string parseMsg(const zmq::message_t& msg) {
        /*
         * Visitor used to unfold the hierarchy of an unknown data structure,
         */
        struct visitor {
            std::string& m_s;
            bool m_ref;

            explicit visitor(std::string& s):m_s(s), m_ref(false) {}
            ~visitor() { m_s += "\n"; }

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
                m_s += '"' + std::string(v, size) + '"';
                return true;
            }
            bool visit_bin(const char* v, uint32_t size) {
                if (is_key_)
                    m_s += std::string(v, size);
                else m_s += "(bin)";
                return true;
            }
            bool visit_ext(const char* /*v*/, uint32_t /*size*/) {
                return true;
            }
            bool start_array_item() {
                return true;
            }
            bool start_array(uint32_t /*size*/) {
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
                tracker_.push(level_++);
                return true;
            }
            bool start_map_key() {
                is_key_ = true;
                m_s += "\n";

                for (int i=0; i< tracker_.top(); ++i) m_s += "    ";
                return true;
            }
            bool end_map_key() {
                m_s += ": ";
                is_key_ = false;
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
                tracker_.pop();
                --level_;
                return true;
            }
            void parse_error(size_t /*parsed_offset*/, size_t /*error_offset*/) {
                std::cerr << "parse error"<<std::endl;
            }
            void insufficient_bytes(size_t /*parsed_offset*/, size_t /*error_offset*/) {
                std::cout << "insufficient bytes" << std::endl;
            }

            // These two functions are required by parser.
            void set_referenced(bool ref) { m_ref = ref; }
            bool referenced() const { return m_ref; }

        private:
            std::stack<int> tracker_;
            uint16_t level_ = 0;
            bool is_key_ = false;
        };

        std::string data_str;
        visitor vst(data_str);
    #ifdef DEBUG
        assert(msgpack::parse(static_cast<const char*>(msg.data()), msg.size(), visitor));
    #else
        msgpack::parse(static_cast<const char*>(msg.data()), msg.size(), vst);
    #endif
        return data_str;
    }

    /*
     * Parse a multipart message packed by msgpack using "visitor".
     */
    static std::string parseMultipartMsg(const MultipartMsg& mpmsg, bool boundary=true) {
        std::string output;
        std::string separator("\n----------new message----------\n");
        for (auto& msg : mpmsg) {
            if (boundary) output.append(separator);
            output.append(parseMsg(msg));
        }
        return output;
    }

    /*
     * Add formatted output to a stringstream.
     */
    template <typename T>
    void prettyStream(const std::pair<std::string, T>& v, std::stringstream& ss) {
        ss << v.first
           << ", " << v.second.containerType()
           << ", " << vectorToString(v.second.shape())
           << ", " << v.second.dtype();

        if (v.second.containerType() == "map" || v.second.containerType() == "MSGPACK_OBJECT_EXT")
            ss << " (Check...unexpected data type!)";
        ss << "\n";
    }

public:
    /*
     * Constructor.
     *
     * @param timeout: connection timeout in second. "-1." (default) for infinite.
     */
    explicit Client(double timeout=-1.): ctx_(1), socket_(ctx_, ZMQ_REQ) {
      socket_.setsockopt(ZMQ_RCVTIMEO, timeout < 0 ? -1 : static_cast<int>(1000 * timeout));
      socket_.setsockopt(ZMQ_LINGER, 0);
    }

    // The destructor of zmq::context_t calls 'zmq_ctx_destroy'.
    // The destructor of zmq::socket_t calls 'zmq_close'.
    ~Client() = default;

    // The copy and copy assignment constructor are implicitly deleted since
    // those of zmq::context_t and zmq::socket_t are deleted.
    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;

    void connect(const std::string& endpoint) {
        std::cout << "Connecting to server: " << endpoint << std::endl;
        socket_.connect(endpoint);
    }

    /*
     * Request and return the next data from the server.
     *
     * Exceptions:
     * std::runtime_error if unexpected message number or unknown "content" is found
     */
    std::map<std::string, kb_data> next() {
        std::map<std::string, kb_data> data_pkg;

        if (!recv_ready_) {
            sendRequest();
            recv_ready_ = true;
        }

        MultipartMsg mpmsg;
        try {
            mpmsg = receiveMultipartMsg();
            recv_ready_ = false;
        } catch (const ZmqTimeoutError&) {
            return data_pkg;
        }

        if (mpmsg.empty()) return data_pkg;

        if (mpmsg.size() % 2)
            throw std::runtime_error(
                "The multipart message is expected to contain (header, data) pairs!");

        kb_data kbdt;

        std::string source;
        bool is_initialized = false;
        auto it = mpmsg.begin();
        while(it != mpmsg.end()) {
            // the header must contain "source" and "content"
            msgpack::object_handle oh_header;
            msgpack::unpack(oh_header, static_cast<const char*>(it->data()), it->size());
            auto header_unpacked = oh_header.get().as<ObjectMap>();

            auto content = header_unpacked.at("content").as<std::string>();

            // the next message is the content (data)
            if (content == "msgpack") {
                if (!is_initialized)
                    is_initialized = true;
                else {
                    data_pkg.insert(std::make_pair(source, std::move(kbdt)));
                    // TODO: the following 'swap" seems to be redundant
                    kb_data empty_data;
                    kbdt.swap(empty_data);
                }

                kbdt.appendMsg(std::move(*it));
                std::advance(it, 1);

                msgpack::object_handle oh_data;
                msgpack::unpack(oh_data, static_cast<const char*>(it->data()), it->size());
                kbdt.metadata = header_unpacked.at("metadata").as<ObjectMap>();

                auto data_unpacked = oh_data.get().as<ObjectMap>();
                for (auto& v : data_unpacked) kbdt.insert(v); // shallow copy

                kbdt.appendHandle(std::move(oh_header));
                kbdt.appendHandle(std::move(oh_data));

            } else if ((content == "array" || content == "ImageData")) {
                kbdt.appendMsg(std::move(*it));
                std::advance(it, 1);

                auto tmp = header_unpacked.at("shape").as<std::vector<unsigned int>>();
                std::vector<std::size_t> shape(tmp.begin(), tmp.end());
                auto dtype = header_unpacked.at("dtype").as<std::string>();
                toCppTypeString(dtype);

                kbdt.array.insert(std::make_pair(header_unpacked.at("path").as<std::string>(),
                                                 NDArray(it->data(), shape, dtype)));
            } else {
                throw std::runtime_error("Unknown data content: " + content);
            }

            source = header_unpacked.at("source").as<std::string>();

            kbdt.appendMsg(std::move(*it));
            std::advance(it, 1);
        }

        data_pkg.insert(std::make_pair(source, std::move(kbdt)));
        kb_data empty_data;
        kbdt.swap(empty_data);

        return data_pkg;
    }

    /*
     * Parse the next multipart message.
     *
     * Note:: this member function consumes data!!!
     */
    std::string showMsg() {
        sendRequest();
        auto mpmsg = receiveMultipartMsg();
        return parseMultipartMsg(mpmsg);
    }

    /*
     * Parse the data structure of the received kb_data.
     *
     * Note:: this member function consumes data!!!
     */
    std::string showNext() {
        auto data_pkg = next();

        std::stringstream ss;
        for (auto& data : data_pkg) {
            ss << "source: " << data.first << "\n";
            ss << "Total bytes received: " << data.second.bytesReceived() << "\n\n";

            ss << "path, container, container shape, type\n";

            ss << "\nmetadata\n" << std::string(8, '-') << "\n";
            for (auto& v : data.second.metadata) prettyStream<MsgpackObject>(v, ss);

            ss << "\ndata\n" << std::string(4, '-') << "\n";
            for (auto& v : data.second) prettyStream<MsgpackObject>(v, ss);

            ss << "\narray\n" << std::string(5, '-') << "\n";
            for (auto& v : data.second.array) prettyStream<NDArray>(v, ss);

            ss << "\n";
        }

        return ss.str();
    }
};

} // karabo_bridge

#endif //KARABO_BRIDGE_KB_CLIENT_HPP
