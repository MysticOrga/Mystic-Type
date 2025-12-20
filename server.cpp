/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** main
*/

#include "src/Network/SessionManager.hpp"
#include "src/Network/TransportLayer/TCP/TCPServer.hpp"
#include "src/Network/TransportLayer/UDP/UDPGameServer.hpp"

#include <iostream>
#include <thread>

int main()
{
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        return 1;
    }
#endif
    try
    {
        SessionManager sessions;
        TCPServer tcpServer(4243, sessions);
        UDPGameServer udpServer(4243, sessions, 50); // ~20 snapshots/s

        std::thread tcpThread([&tcpServer]() { tcpServer.run(); });

        udpServer.run();

        tcpThread.join();
    }
    catch (const std::exception &e)
    {
        std::cerr << "[SERVER IOERROR] " << e.what() << std::endl;
    }
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
