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
        TCPServer tcpServer(4243);
        // Snapshots plus fréquents pour réduire la latence d’apparition des tirs
        UDPGameServer udpServer(4243, 50); // ~20 snapshots/s

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
