/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** UDP game server entry point (process enfant)
*/

#include "Network/TransportLayer/UDP/UDPGameServer.hpp"
#include "Network/SessionManager.hpp"
#include "server/IpcChannel.hpp"

#include <iostream>
#include <string>
#include <optional>
#include <cstdlib>
#include <sstream>
#include <thread>

namespace {
    struct Args {
        std::string lobby = "PUBLIC";
        uint16_t port = 4244;
        std::string ipcSock;
    };

    std::string logPrefix(const Args &args)
    {
        std::ostringstream oss;
        oss << "[UDP lobby=" << args.lobby << " port=" << args.port << "] ";
        return oss.str();
    }

    std::optional<Args> parseArgs(int argc, char **argv)
    {
        Args args;
        for (int i = 1; i < argc; ++i) {
            std::string a = argv[i];
            if (a == "--lobby" && i + 1 < argc) {
                args.lobby = argv[++i];
            } else if (a == "--udp-port" && i + 1 < argc) {
                args.port = static_cast<uint16_t>(std::atoi(argv[++i]));
            } else if (a == "--ipc-sock" && i + 1 < argc) {
                args.ipcSock = argv[++i];
            }
        }
        return args;
    }
}

int main(int argc, char **argv)
{
    auto argsOpt = parseArgs(argc, argv);
    if (!argsOpt.has_value()) {
        std::cerr << "[UDP SERVER ERROR] Failed to parse args\n";
        return 1;
    }
    Args args = *argsOpt;

    IpcChannel ipc;
    if (!args.ipcSock.empty()) {
        if (!ipc.connectClient(args.ipcSock)) {
            std::cerr << logPrefix(args) << "Unable to connect IPC at " << args.ipcSock << "\n";
            return 1;
        }
        ipc.send("READY");
    }

    try {
        SessionManager sessions;
        UDPGameServer udpServer(args.port, sessions, 50, args.lobby);
        if (!args.ipcSock.empty()) {
            udpServer.setIpc(&ipc);
        }
        std::cout << logPrefix(args) << "Started\n";
        udpServer.run();
    }
    catch (const std::exception &e) {
        std::cerr << "[UDP SERVER ERROR] " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
