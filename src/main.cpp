/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** main
*/

#include "Network/TransportLayer/TCP/TCPServer.hpp"

#include <iostream>

int main()
{
    try {
        TCPServer server(4242);
        server.run();
    }
    catch (const std::exception &e) {
        std::cerr << "[SERVER ERROR] " << e.what() << std::endl;
    }
    return 0;
}
