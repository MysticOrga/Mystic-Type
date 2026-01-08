/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** Child process manager to spawn/track UDP game servers
*/

#pragma once

#include <string>
#include <unordered_map>
#include <optional>

struct ChildInfo {
    int pid = -1;
    std::string lobbyCode;
    uint16_t udpPort = 0;
    std::string ipcSock;
};

/**
 * @brief Minimal child process manager to spawn UDP game servers.
 *
 * This is a stub: no restart logic, no signal handling, just spawn and track.
 */
class ChildProcessManager {
public:
    explicit ChildProcessManager(std::size_t maxChildren = 64) : _maxChildren(maxChildren) {}

    /**
     * @brief Spawn a UDP game server process.
     * @param lobby lobby code
     * @param udpPort udp port to bind
     * @param ipcSock path to IPC socket
     * @return pid on success, -1 on failure
     */
    int spawn(const std::string &lobby, uint16_t udpPort, const std::string &ipcSock);

    /**
     * @brief Retrieve info by lobby code.
     */
    std::optional<ChildInfo> get(const std::string &lobby) const;

    /**
     * @brief Remove tracking info (does not kill).
     */
    void forget(const std::string &lobby);

    std::size_t activeCount() const { return _children.size(); }
    std::size_t maxCount() const { return _maxChildren; }

private:
    std::unordered_map<std::string, ChildInfo> _children;
    std::size_t _maxChildren;
};
