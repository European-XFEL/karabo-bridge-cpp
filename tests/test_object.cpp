//
// Author: Jun Zhu, zhujun981661@gmail.com
//
#include "kb_client.hpp"


template<typename T>
karabo_bridge::Object packObject(T x) {
    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, x);
    msgpack::object_handle oh = msgpack::unpack(sbuf.data(), sbuf.size());
    return oh.get().as<karabo_bridge::Object>();
}

karabo_bridge::Object packBin(std::vector<int> x) {
    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, msgpack::type::raw_ref(reinterpret_cast<const char*>(x.data()),
                                               x.size() * sizeof(int)));
    msgpack::object_handle oh = msgpack::unpack(sbuf.data(), sbuf.size());
    return oh.get().as<karabo_bridge::Object>();
}

int main() {
    auto obj_int = packObject<int>(10);
    assert(obj_int.size() == 1);
    assert(obj_int.dtype() == "uint64_t");
    assert(obj_int.subType() == "uint64_t");
    assert(obj_int.as<uint64_t>() == 10);

    std::vector<int> vec({1, 2, 3, 4});
    auto obj_arr = packObject(vec);
    assert(obj_arr.size() == 4);
    assert(obj_arr.dtype() == "MSGPACK_OBJECT_ARRAY");
    assert(obj_arr.subType() == "uint64_t");
    assert(obj_arr.as<std::vector<int>>() == vec);

    auto obj_map = packObject<std::map<int, int>>({{1, 2}, {3, 4}});
    assert(obj_map.size() == 2);
    assert(obj_map.dtype() == "MSGPACK_OBJECT_MAP");
    assert(obj_map.subType() == "unimplemented");

    auto obj_bin = packBin(std::vector<int>({1, 2, 3, 4}));
    assert(obj_bin.size() == 16);
    assert(obj_bin.dtype() == "MSGPACK_OBJECT_BIN");
    assert(obj_bin.subType() == "byte");
}
