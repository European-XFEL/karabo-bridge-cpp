//
// Author: Jun Zhu, zhujun981661@gmail.com
//
#include "karabo-bridge/kb_client.hpp"


template<typename T>
karabo_bridge::MsgpackObject packObject(T x) {
    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, x);
    msgpack::object_handle oh = msgpack::unpack(sbuf.data(), sbuf.size());
    return oh.get().as<karabo_bridge::MsgpackObject>();
}

int main() {
    auto obj1 = packObject<int>(100);
    auto obj2 = packObject<float>(0.002);
    karabo_bridge::ObjectMap obj_data{{"obj1", obj1}, {"obj2", obj2}};

    uint16_t a[12] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    karabo_bridge::NDArray arr1((void*)a, std::vector<std::size_t>{2, 2, 3}, "uint16_t");

    karabo_bridge::kb_data data;

    // copy from obj_data
    for (auto &v : obj_data) { data.insert(v); }
    assert(obj_data["obj1"].as<int>() == 100);

    // check iterator
    auto it = data.begin();
    assert(it->first == "obj1");
    assert(it->second.as<uint64_t>() == 100);
    ++it;
    assert(it->first == "obj2");
    assert(it->second.as<float>() - 0.002 < 1e-10);
    ++it;
    assert(it == data.end());

    // access via iterator
    for (auto it = data.begin(); it != data.end(); ++it)
        std::cout << it->first << std::endl;

    // access using auto range
    for (auto &v : data) std::cout << v.first << std::endl;
}
