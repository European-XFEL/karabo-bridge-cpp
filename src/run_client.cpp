#include "kb_client.hpp"

#include <iostream>


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
