/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** main
*/

#include "src/Network/SessionManager.hpp"
#include "src/Network/TransportLayer/TCP/TCPServer.hpp"
#include "src/Network/TransportLayer/UDP/UDPGameServer.hpp"
#include "src/server/ChildProcessManager.hpp"
#include "src/server/IpcChannel.hpp"
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
    try {
        SessionManager sessions;
        ChildProcessManager childMgr;
        TCPServer tcpServer(4243, sessions, &childMgr);
        tcpServer.run();
    }
    catch (const std::exception &e) {
        std::cerr << "[TCP SERVER ERROR] " << e.what() << std::endl;
        return 1;
    }
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
