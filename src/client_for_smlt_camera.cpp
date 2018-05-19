#include "kb_client.hpp"

#include <iostream>
#include <chrono>


template <class T>
void printContainer(const T& container) {
    for (auto v : container) std::cout << v << " ";
    std::cout << std::endl;
}


int main (int argc, char* argv[]) {
    std::string port;
    if (argc >= 2) port = argv[1];
    else throw std::invalid_argument("Port is required!");

    karabo_bridge::Client client;

    client.connect("tcp://localhost:" + port);

    client.showNext();

    auto start = std::chrono::high_resolution_clock::now();

    auto data = client.next();

    assert(data["data.image.bitsPerPixel"].as<uint64_t>() == 32);
    assert(data["data.image.dimensionTypes"].as<std::vector<uint64_t>>() == std::vector<uint64_t>({0, 0}));
    assert(data["data.image.dimensions"].as<std::vector<uint64_t>>() == std::vector<uint64_t>({1024, 1024}));
    assert(data["metadata.timestamp.tid"].as<std::uint64_t>() == 0);

    assert(data.data["data.image.data"].dtype() == "uint32");
    assert(data.data["data.image.data"].shape() == std::vector<int>({1024, 1024}));
    auto image_data = data.data["data.image.data"].as<uint32_t>();

    std::cout << "Passed!" << std::endl;

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Execution time: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
              << " ms" << std::endl;
}
