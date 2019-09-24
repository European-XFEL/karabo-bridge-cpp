/*
 * View the data/message structure received by the bridge.
 *
 * Author: Jun Zhu, zhujun981661@gmail.com
 *
 */
#include "karabo-bridge/kb_client.hpp"

#include <iostream>
#include <chrono>
#include <cstdlib>


int main (int argc, char* argv[]) {
    std::string addr;
    bool show_msg = false;

    if (argc >= 2) {
        addr = argv[1];
        if (argc >=3 && (*argv[2] == 'm' || *argv[2] == 'M')) show_msg = true;
    }
    else throw std::invalid_argument("Server address required!");

    karabo_bridge::Client client;
    client.connect(addr);

    if (show_msg) {
        std::cout << client.showMsg();
    } else {
        auto start = std::chrono::high_resolution_clock::now();
        std::cout << client.showNext();
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "Data acquisition time: " << std::setw(6) << std::fixed << std::setprecision(3)
                  << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000.
                  << " ms\n\n";
    }
}
