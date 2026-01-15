/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** Child process manager to spawn/track UDP game servers
*/

#include "ChildProcessManager.hpp"
#ifndef _WIN32
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/wait.h>
#endif
#include <vector>
#include <cstring>
#include <iostream>

int ChildProcessManager::spawn(const std::string &lobby, uint16_t udpPort, const std::string &ipcSock)
{
#ifdef _WIN32
    (void)lobby;
    (void)udpPort;
    (void)ipcSock;
    return -1;
#else
  pid_t pid = ::fork();
    if (pid < 0) {
        std::cerr << "[PARENT] fork failed\n";
        return -1;
    }
    if (pid == 0) {
        // Child: exec the UDP server binary
        std::string portStr = std::to_string(udpPort);
        std::string exePath = "./rtype-udp-server";
        if (::access(exePath.c_str(), X_OK) != 0) {
            exePath = "rtype-udp-server";
        }
        std::vector<char*> args;
        args.push_back(const_cast<char*>(exePath.c_str()));
        args.push_back(const_cast<char*>("--lobby"));
        args.push_back(const_cast<char*>(lobby.c_str()));
        args.push_back(const_cast<char*>("--udp-port"));
        args.push_back(const_cast<char*>(portStr.c_str()));
        if (!ipcSock.empty()) {
            args.push_back(const_cast<char*>("--ipc-sock"));
            args.push_back(const_cast<char*>(ipcSock.c_str()));
        }
        args.push_back(nullptr);
        ::execv(args[0], args.data());
        // If exec fails
        std::perror("[PARENT] execv failed");
        _exit(1);
    }
    // Parent: track child
    ChildInfo info;
    info.pid = pid;
    info.lobbyCode = lobby;
    info.udpPort = udpPort;
    info.ipcSock = ipcSock;
    _children[lobby] = info;
    std::cout << "[PARENT] Spawned UDP server lobby=" << lobby
              << " port=" << udpPort << " pid=" << pid << "\n";
    return pid;
#endif

}

std::optional<ChildInfo> ChildProcessManager::get(const std::string &lobby) const
{
    auto it = _children.find(lobby);
    if (it == _children.end())
        return std::nullopt;
    return it->second;
}

void ChildProcessManager::forget(const std::string &lobby)
{
    _children.erase(lobby);
}
