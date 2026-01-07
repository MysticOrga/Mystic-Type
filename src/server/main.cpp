/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** main
*/

#include "Network/TransportLayer/TCP/TCPServer.hpp"
#include "Network/SessionManager.hpp"

#include <iostream>

int main()
{
    try {
        SessionManager sessions;
        TCPServer tcpServer(4243, sessions);
        tcpServer.run();
    }
    catch (const std::exception &e) {
        std::cerr << "[TCP SERVER ERROR] " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
