#include "kb_client.hpp"

#include <iostream>
#include <chrono>


template <class T>
void printContainer(const T& container) {
    for (auto v : container) std::cout << v << " ";
    std::cout << std::endl;
}


int main (int argc, char* argv[]) {
    std::string port = "1234";
    if (argc >= 2) port = argv[1];

    karabo_bridge::Client client;

    client.connect("tcp://localhost:" + port);

    client.showNext();

    auto start = std::chrono::high_resolution_clock::now();

    auto data = client.next();

    assert(data["header.pulseCount"].as<uint64_t>() == 32);
    assert(data["trailer.status"].as<uint64_t>() == 0);
    assert(data["header.majorTrainFormatVersion"].as<uint64_t>() == 2);
    assert(data["header.minorTrainFormatVersion"].as<uint64_t>() == 1);

    assert(data.data["image.trainId"].dtype() == "uint64");
    assert(data.data["image.trainId"].shape() == std::vector<int>{32});
    auto train_id = data.data["image.trainId"].as<uint64_t>();
    for (auto v : train_id) assert(v >= 10000000000);
    assert(train_id.size() == 32);
 
    assert(data.data["detector.data"].dtype() == "uint8");
    assert(data.data["detector.data"].shape() == std::vector<int>{416});
    auto dt_data = data.data["detector.data"].as<uint8_t>();
    for (auto v : dt_data) assert(static_cast<unsigned int>(v) == 1);
    assert(dt_data.size() == 416);

    std::cout << "Passed!" << std::endl;

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Execution time: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
              << " ms" << std::endl;
}
