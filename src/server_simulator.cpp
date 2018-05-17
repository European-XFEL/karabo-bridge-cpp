#include "kb_client.hpp"

#include <string>
#include <iostream>
#include <sstream>
#include <array>
#include <unistd.h>


template <class T>
void printContainer(const T& container) {
    for (auto v : container) std::cout << v << " ";
    std::cout << std::endl;
}


int main () {
    karabo_bridge::Client client;

    client.connect("tcp://localhost:1234");

    client.showNext();

    auto result = client.next();

    assert(result.metadata["source"].as<std::string>() == "SPB_DET_AGIPD1M-1/DET/detector");

    std::cout << "timestamp.tid: " << result.metadata["timestamp.tid"].as<uint64_t>() << "\n";
    std::cout << "timestamp.sec: " << result.metadata["timestamp.sec"].as<std::string>() << "\n";
    std::cout << "timestamp.frac: " << result.metadata["timestamp.frac"].as<std::string>() << "\n";

    assert(result["header.pulseCount"].as<uint64_t>() == 32);
    assert(result["trailer.status"].as<uint64_t>() == 0);
    assert(result["header.majorTrainFormatVersion"].as<uint64_t>() == 2);
    assert(result["header.minorTrainFormatVersion"].as<uint64_t>() == 1);

    auto train_id = result["image.trainId"].asArray<uint64_t, 32>();
    printContainer(train_id);
}
