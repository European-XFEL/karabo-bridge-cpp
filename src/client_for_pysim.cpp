/*
 * A test use the simulation server from
 *      https://github.com/European-XFEL/karabo-bridge-py.git
 *
 * To run the server, use
 *
 *      from karabo_bridge import start_gen
 *      start_gen(1234)
 *
 * Author: Jun Zhu, zhujun981661@gmail.com
 *
 */
#include "kb_client.hpp"

#include <iostream>
#include <thread>
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

    std::cout << client.showMsg() << "\n";
    std::cout << client.showNext() << "\n";

    for (int i=0; i<10; ++i) {
        // there is bottleneck in the server side
        std::this_thread::sleep_for(std::chrono::milliseconds(400));

        auto start = std::chrono::high_resolution_clock::now();
        auto data_pkg = client.next();
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "Run " << std::setw(2) << i+1
                  << ", data processing time: " << std::fixed << std::setprecision(1)
                  << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000.
                  << " ms\n";

        karabo_bridge::kb_data data(std::move(data_pkg.begin()->second));
        assert(data_pkg.begin()->first == "SPB_DET_AGIPD1M-1/DET/detector");

        assert(data["header.pulseCount"].as<uint64_t>() == 31); //32);
        assert(data["trailer.status"].as<uint64_t>() == 0);
        assert(data["header.majorTrainFormatVersion"].as<uint64_t>() == 2);
        assert(data["header.minorTrainFormatVersion"].as<uint64_t>() == 1);

        // TODO: check msgpack::object_type::BIN data

        assert(data.array["image.trainId"].dtype() == "uint64_t");
        assert(data.array["image.trainId"].shape() == std::vector<unsigned int>{31});
        auto train_id = data.array["image.trainId"].as<uint64_t>();
        for (auto v : train_id) assert(v >= 10000000000);
        assert(train_id.size() == 31);

        assert(data.array["image.data"].dtype() == "uint16_t");
        assert(data.array["image.data"].shape()[0] == 31);
        assert(data.array["image.data"].shape()[1] == 16);
        assert(data.array["image.data"].shape()[2] == 512);
        assert(data.array["image.data"].shape()[3] == 128);
        auto image_data = data.array["image.data"].as<uint16_t>();
        for (auto v : image_data) assert(v >= 1500 && v <= 1600);
    }

    std::cout << "Passed!" << std::endl;
}
