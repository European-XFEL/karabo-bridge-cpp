/*
 * The client connects to the simulation server implemented in
 *      https://github.com/European-XFEL/karabo-bridge-py.git
 *
 * To start a server, run the following command in the terminal
 *
 *      karabo-bridge-server-sim 1234 -n 2
 *
 * Author: Jun Zhu, zhujun981661@gmail.com
 *
 */
#include "karabo-bridge/kb_client.hpp"

#include <iostream>
#include <thread>
#include <chrono>


int main (int argc, char* argv[]) {
    std::string addr = "localhost:1234";
    if (argc >= 2) addr = argv[1];

    karabo_bridge::Client client;

    client.connect("tcp://" + addr);

    std::cout << client.showMsg() << "\n";
    std::cout << client.showNext() << "\n";

    for (int i = 0; i < 5; ++i) {
        // there is bottleneck in the server side
        std::this_thread::sleep_for(std::chrono::milliseconds(400));

        auto start = std::chrono::high_resolution_clock::now();
        auto data_pkg = client.next();
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "Run " << std::setw(2) << i + 1
                  << ", data acquisition time: " << std::fixed << std::setprecision(1)
                  << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000.
                  << " ms\n";

        int ns = 1; // The No. of source
        for (auto it = data_pkg.begin(); it != data_pkg.end(); ++it) {
            karabo_bridge::kb_data data(std::move(it->second));

            if (data_pkg.size() == 1) {
                assert(it->first == "SPB_DET_AGIPD1M-1/CAL/APPEND_CORRECTED");
                assert(data.metadata["source"].as<std::string>() == it->first);
            } else {
                assert(it->first == "SPB_DET_AGIPD1M-1/CAL/APPEND_CORRECTED-" + std::to_string(ns++));
                assert(data.metadata["source"].as<std::string>() == it->first);
            }

            assert(data.metadata["timestamp.tid"].as<uint64_t>() >= 10000000000);
            data.metadata["timestamp.frac"].as<std::string>();

            auto image_passport = data["image.passport"].as<std::vector<std::string>>();

            assert(data["modulesPresent"].dtype() == "bool");
            assert(data["modulesPresent"].shape() == std::vector<std::size_t>{16});
            auto modules_present = data["modulesPresent"].as<std::vector<bool>>();
            for (auto v : modules_present) assert(v == true);

            assert(data["sources"].dtype() == "string");
            assert(data["sources"].shape() == std::vector<std::size_t>{16});
            auto sources = data["sources"].as<std::vector<std::string>>();
            for (int i=0; i<16; ++i)
                assert(sources[i] == "SPB_DET_AGIPD1M-1/CAL/" + std::to_string(i) + "CH0:xtdf");

            assert(data.array["image.cellId"].dtype() == "uint16_t");
            assert(data.array["image.cellId"].shape() == std::vector<std::size_t>{64});
            auto cell_id = data.array["image.cellId"].as<std::array<uint16_t, 64>>();
            for (std::size_t i_id=0; i_id < cell_id.size(); ++i_id) assert(i_id == cell_id[i_id]);
            assert(cell_id.size() == 64);

            assert(data.array["image.data"].dtype() == "float");
            assert(data.array["image.data"].shape()[0] == 16);
            assert(data.array["image.data"].shape()[1] == 128);
            assert(data.array["image.data"].shape()[2] == 512);
            assert(data.array["image.data"].shape()[3] == 64);
            auto image_data = data.array["image.data"].as<std::vector<float>>();
            for (auto& v : image_data) { assert(v >= 1500 && v <= 1600); }
            auto ptr = data.array["image.data"].data<float>();
            for (auto ptr_end = ptr + data.array["image.data"].size(); ptr != ptr_end; ++ptr) {
                assert(*ptr >= 1500 && *ptr <= 1600);
            }
        }
    }

    std::cout << "Passed!" << std::endl;
}
