/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** main
*/

#include "Network/TransportLayer/TCP/TCPServer.hpp"
#include "Network/SessionManager.hpp"
#include "server/ChildProcessManager.hpp"
#include "server/IpcChannel.hpp"

#include <iostream>
#include <string>

// Parent server entry point: TCP matchmaking only.
int main()
{
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
    return 0;
}
