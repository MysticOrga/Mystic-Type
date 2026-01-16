/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** Child process manager to spawn/track UDP game servers
*/

#include "ChildProcessManager.hpp"
#ifdef _WIN32
    #include <Windows.h>
#else
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/wait.h>
    #include <netinet/in.h>
#endif
#include <vector>
#include <cstring>
#include <iostream>

int ChildProcessManager::spawn(const std::string &lobby, uint16_t udpPort, const std::string &ipcSock)
{
    std::string portStr = std::to_string(udpPort);
    std::vector<char *> args;

    ChildInfo info;
#ifdef _WIN32
    std::string exePath = "./rtype-udp-server.exe";
    STARTUPINFO si = {sizeof(si)};
    PROCESS_INFORMATION pi = {0};
#else
    std::string exePath = "./rtype-udp-server";
#endif // _WIN32

    args.push_back(const_cast<char *>(exePath.c_str()));
    args.push_back(const_cast<char *>("--lobby"));
    args.push_back(const_cast<char *>(lobby.c_str()));
    args.push_back(const_cast<char *>("--udp-port"));
    args.push_back(const_cast<char *>(portStr.c_str()));
    if (!ipcSock.empty())
    {
        args.push_back(const_cast<char *>("--ipc-sock"));
        args.push_back(const_cast<char *>(ipcSock.c_str()));
    }
    args.push_back(nullptr);
#ifdef _WIN32
    std::string commandLine;

    for (size_t i = 0; i < args.size(); ++i)
    {
        if (!args[i])
            continue;

        // Quote arguments containing spaces
        if (std::strchr(args[i], ' '))
        {
            commandLine += "\"";
            commandLine += args[i];
            commandLine += "\"";
        }
        else
        {
            commandLine += args[i];
        }

        if (i + 1 < args.size())
            commandLine += " ";
    }

    // CreateProcess requires writable buffer
    std::vector<char> cmdBuffer(commandLine.begin(), commandLine.end());
    cmdBuffer.push_back('\0');

    BOOL ok = CreateProcessA(NULL,             // Application name
                             cmdBuffer.data(), // Command line (modifiable)
                             NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

    if (!ok)
    {
        std::cerr << "[PARENT] CreateProcess failed: " << GetLastError() << "\n";
        return -1;
    }

    info.pid = pi.dwProcessId;

    // Close handles (Windows does not auto-clean)
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
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
    info.pid = pid;

    // Parent: track child
#endif
    info.lobbyCode = lobby;
    info.udpPort = udpPort;
    info.ipcSock = ipcSock;
    _children[lobby] = info;
    std::cout << "[PARENT] Spawned UDP server lobby=" << lobby
              << " port=" << udpPort << " pid=" << info.pid << "\n";
    return info.pid;

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
