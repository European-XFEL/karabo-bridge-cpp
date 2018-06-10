//
// Author: Jun Zhu, zhujun981661@gmail.com
//
#include <cassert>
#include "kb_client.hpp"


using namespace karabo_bridge;


template<typename T>
void test_msgpack_object(T x, bool test_throw=false) {
    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, x);
    msgpack::object_handle oh = msgpack::unpack(sbuf.data(), sbuf.size());
    Object obj = oh.get().as<Object>();

    AnyObject any(obj);

    if (!test_throw) {
        assert((any.as<Object, T>() == x));
    } else {
        try {
            any.as<msgpack::object, T>();
        } catch (const bad_any_cast& e) {
            return;
        }
        assert(false);
    }
}
//
//void test_array() {
//    std::vector<int> vec {1, 2, 3, 4};
//    Array arr(vec.data(), {4}, "int");
//
//    AnyObject any(arr);
//
//    std::vector<int> x = any.as<Array, std::vector<int>>();
//    for (auto v : x) std::cout << v << ", ";
//}

int main() {
    test_msgpack_object<int>(10);
    test_msgpack_object<std::string>("any object");
    test_msgpack_object<std::vector<int>>({1, 2, 3, 4});

    test_msgpack_object<std::string>("any object", true);
}

