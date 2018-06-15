//
// Author: Jun Zhu, zhujun981661@gmail.com
//
#include "kb_client.hpp"


int main() {
    uint16_t a[12] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

    karabo_bridge::NDArray array_uint16((void*)a, std::vector<std::size_t>{2, 2, 3}, "uint16_t");
    assert(array_uint16.dtype() == "uint16_t");
    assert(array_uint16.shape() == std::vector<std::size_t>({2, 2, 3}));
    assert(array_uint16.size() == 12);
    assert(array_uint16.containerType() == "array-like");
    assert(*static_cast<uint16_t*>(array_uint16.data()) - 1 < 1e-10);
    assert(*array_uint16.data<uint16_t>() - 1 < 1e-10);
    assert(array_uint16.as<std::vector<uint16_t>>()
           == std::vector<uint16_t>({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12}));
    assert(array_uint16.as<std::deque<uint16_t>>()
           == std::deque<uint16_t>({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12}));
    assert((array_uint16.as<std::array<uint16_t, 12>>()
           == std::array<uint16_t, 12>({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12})));

    try {
        array_uint16.as<std::vector<int>>();
        assert(false);
    } catch (karabo_bridge::TypeMismatchErrorNDArray& e) {
        std::cout << e.what() << std::endl;
    }

    try {
        array_uint16.as<std::map<int, int>>();
        assert(false);
    } catch (karabo_bridge::TypeMismatchErrorNDArray& e) {
        std::cout << e.what() << std::endl;
    }

    // array_uint16.as<uint16_t>();  // won't compile, no matching function

    try {
        array_uint16.data<int>();
        assert(false);
    } catch (karabo_bridge::TypeMismatchErrorNDArray& e) {
        std::cout << e.what() << std::endl;
    }

    try {
        array_uint16.as<std::array<uint16_t, 13>>();
        assert(false);
    } catch (karabo_bridge::CastErrorNDArray& e) {
        std::cout << e.what() << std::endl;
    }
}
