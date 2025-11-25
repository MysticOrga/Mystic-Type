/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** main
*/

#include "Network/TransportLayer/TCP/TCPServer.hpp"
#include "Network/TransportLayer/UDP/UDPGameServer.hpp"

#include <iostream>
#include <thread>

int main()
{
    try {
        TCPServer tcpServer(4242);
        UDPGameServer udpServer(4242, 500); // snapshot every 500 ms

        std::thread tcpThread([&tcpServer]() {
            tcpServer.run();
        });

        udpServer.run();

        tcpThread.join();
    }
    catch (const std::exception &e) {
        std::cerr << "[SERVER ERROR] " << e.what() << std::endl;
    }
    return 0;
}
