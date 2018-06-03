/*
 * A test use a LimaSimulatedCamera and PipeToZeroMQ devices in local Karabo
 * environment.
 *
 * Author: Jun Zhu, zhujun981661@gmail.com
 *
 */
#include "kb_client.hpp"

#include <iostream>
#include <thread>
#include <chrono>


int main (int argc, char* argv[]) {
    std::string port;
    if (argc >= 2) port = argv[1];
    else throw std::invalid_argument("Port is required!");

    karabo_bridge::Client client;

    client.connect("tcp://localhost:" + port);

    std::cout << client.showMsg() << "\n";
    std::cout << client.showNext() << "\n";

    for (int i=0; i<10; ++i) {
        // there is bottleneck in the server side
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        auto start = std::chrono::high_resolution_clock::now();
        auto data_pkg = client.next();
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "Run " << std::setw(2) << i+1
                  << ", data processing time: " << std::setw(6) << std::fixed << std::setprecision(3)
                  << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000.
                  << " ms";

        karabo_bridge::kb_data data(std::move(data_pkg.begin()->second));
        assert(data_pkg.begin()->first == "camera:output");

        assert(data["data.image.bitsPerPixel"].as<uint64_t>() == 32);
        assert(data["data.image.dimensionTypes"].as<std::vector<uint64_t>>() == std::vector<uint64_t>({0, 0}));
        assert(data["data.image.dimensions"].as<std::vector<uint64_t>>() == std::vector<uint64_t>({1024, 1024}));
        assert(data["metadata.timestamp.tid"].as<std::uint64_t>() == 0);

        assert(data.array["data.image.data"].dtype() == "uint32_t");
        assert(data.array["data.image.data"].shape() == std::vector<unsigned int>({1024, 1024}));
        start = std::chrono::high_resolution_clock::now();
        auto image_data = data.array["data.image.data"].as<uint32_t>();
        end = std::chrono::high_resolution_clock::now();
        std::cout << ", time for processing a 1024x1024 uint32_t image data: "
                  << std::fixed << std::setprecision(3)
                  << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000.
                  << " ms\n";
        assert(image_data.size() == 1024*1024);
        // The number increases with time, so one should restart the camera
        // after several runs.
        for (auto v : image_data) assert(v >= 0 && v <= 50000);
    }

    std::cout << "Passed!" << std::endl;
}
