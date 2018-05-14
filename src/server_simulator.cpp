#include "kb_client.hpp"

#include <zmq.hpp>

#include <string>
#include <iostream>
#include <sstream>
#include <array>
#include <unistd.h>


int main () {
    //  Prepare our context and socket
    zmq::context_t context (1);
    zmq::socket_t socket (context, ZMQ_REP);
    socket.bind ("tcp://*:1234");

    while (1) {
        std::cout << "Waiting for request...\n";
        zmq::message_t request;
        //  Wait for next request from client
        socket.recv(&request);
        std::string req_str;
        karabo_bridge::msg2str(request, req_str);
        if (req_str != "next") {
            std::cout << "Unknown request: " << req_str << std::endl;
            continue;
        }

        // Send a multipart message

        // strings
        zmq::message_t tag_msg(17);
        memcpy(tag_msg.data(), "multipart message", 17);
        socket.send(tag_msg, ZMQ_SNDMORE);

        // array
        std::array<int, 5> arr {1, 2, 3, 4, 5};
        msgpack::sbuffer sbuf;
        msgpack::pack(sbuf, arr);

        zmq::message_t data_msg(sbuf.size());
        memcpy(data_msg.data(), sbuf.data(), sbuf.size());
        socket.send(data_msg, 0);
    }
}
