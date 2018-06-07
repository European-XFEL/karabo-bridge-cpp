/*
 * A tool to view the data structure coming from the server
 *
 * To run, for example, type
 *
 *     glimpse tcp://localhost:1234
 *
 * Author: Jun Zhu, zhujun981661@gmail.com
 *
 */
#include "kb_client.hpp"

#include <iostream>
#include <thread>
#include <chrono>


int main (int argc, char* argv[]) {
    std::string addr;
    if (argc >= 2) addr = argv[1];
    else throw std::invalid_argument("Server address required!");

    karabo_bridge::Client client;

    client.connect(addr);

    while (true) {
        // there is bottleneck in the server side
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        auto start = std::chrono::high_resolution_clock::now();
        std::cout << client.showNext();
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "Data acquiring time: " << std::setw(6) << std::fixed << std::setprecision(3)
                  << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000.
                  << " ms\n\n";

    }
}
