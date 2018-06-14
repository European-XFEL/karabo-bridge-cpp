//
// Author: Jun Zhu, zhujun981661@gmail.com
//
#include "kb_client.hpp"


template<typename T>
karabo_bridge::MsgpackObject packObject(T x) {
    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, x);
    msgpack::object_handle oh = msgpack::unpack(sbuf.data(), sbuf.size());
    return oh.get().as<karabo_bridge::MsgpackObject>();
}

karabo_bridge::MsgpackObject packBin(std::vector<int> x) {
    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, msgpack::type::raw_ref(reinterpret_cast<const char*>(x.data()),
                                               x.size() * sizeof(int)));
    msgpack::object_handle oh = msgpack::unpack(sbuf.data(), sbuf.size());
    return oh.get().as<karabo_bridge::MsgpackObject>();
}

int main() {
    auto obj_uint = packObject<std::size_t>(2147483648);
    assert(obj_uint.size() == 0);
    assert(obj_uint.dtype() == "uint64_t");
    assert(obj_uint.containerType() == "");
    assert(obj_uint.as<uint64_t>() == 2147483648);
    // Msgpack does the overflow check. So we can give the type check task
    // to msgpack safely.
    try {
        obj_uint.as<int>();
        assert(false);
    } catch (karabo_bridge::CastErrorObject& e) {
        std::cout << e.what() << std::endl;
    }

    auto obj_int = packObject<int>(-10);
    assert(obj_int.size() == 0);
    assert(obj_int.dtype() == "int64_t");
    assert(obj_int.containerType().empty());
    assert(obj_int.as<int64_t>() == -10);

    auto obj_float = packObject<double>(0.00222);
    assert(obj_float.size() == 0);
    assert(obj_float.dtype() == "double");
    assert(obj_float.containerType().empty());
    assert(obj_float.as<double>() - 0.00222 < 1e-10);
    try {
        obj_float.as<int>();
        assert(false);
    } catch (karabo_bridge::CastErrorObject& e) {
        std::cout << e.what() << std::endl;
    }

    auto obj_nil = packObject(msgpack::type::nil_t());
    assert(obj_nil.size() == 0);
    assert(obj_nil.dtype() == "MSGPACK_OBJECT_NIL");
    assert(obj_nil.containerType().empty());
    try {
        obj_nil.as<int>();
        assert(false);
    } catch (karabo_bridge::CastErrorObject& e) {
        std::cout << e.what() << std::endl;
    }

    std::vector<int> vec({1, 2, 3, 4});
    auto obj_arr = packObject(vec);
    assert(obj_arr.size() == 4);
    assert(obj_arr.dtype() == "uint64_t");
    assert(obj_arr.containerType() == "array-like");
    assert(obj_arr.as<std::vector<int>>() == vec);
    assert(obj_arr.as<std::deque<int>>() == std::deque<int>(vec.begin(), vec.end()));
    std::array<int, 4> expected_arr;
    std::copy_n(vec.begin(), 4, expected_arr.begin());
    assert((obj_arr.as<std::array<int, 4>>() == expected_arr));
    try {
        obj_arr.as<double>();
        assert(false);
    } catch (karabo_bridge::CastErrorObject& e) {
        std::cout << e.what() << std::endl;
    }

    auto obj_map = packObject<std::map<int, int>>({{1, 2}, {3, 4}});
    assert(obj_map.size() == 2);
    assert(obj_map.dtype() == "undefined");
    assert(obj_map.containerType() == "map");

    auto obj_bin = packBin(std::vector<int>({1, 2, 3, 4}));
    assert(obj_bin.size() == 16);
    assert(obj_bin.dtype() == "char");
    assert(obj_bin.containerType() == "array-like");
    try {
        obj_bin.as<std::vector<float>>();
        assert(false);
    } catch (karabo_bridge::CastErrorObject& e) {
        std::cout << e.what() << std::endl;
    }
    try {
        obj_bin.as<std::vector<unsigned char>>();
    } catch (karabo_bridge::CastErrorObject& e) {
        throw;
    }
}
