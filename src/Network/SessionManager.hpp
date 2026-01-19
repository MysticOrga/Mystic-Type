/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** Session manager shared between TCP and UDP servers
*/

#pragma once

#ifndef _WIN32
#include <netinet/in.h>
#else
#include <winsock2.h>
#endif
#include <cstdint>
#include <functional>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

/**
 * @brief In-memory session record shared between TCP and UDP servers.
 */
struct Session
{
    int id = 0;
    int tcpFd = -1;
    sockaddr_in tcpAddr{};
    sockaddr_in udpAddr{};
    bool hasUdp = false;
    long lastPong = 0;

    long lastInputMs = 0;
    int inputCount = 0;
    long lastShootMs = 0;
    int shootCount = 0;
    std::string lobbyCode;
    std::string pseudo;
};

/**
 * @brief Tracks client sessions across TCP and UDP, and enforces simple rate limits.
 */
class SessionManager
{
  public:
    /**
     * @brief Construct a manager with simple per-second rate limits.
     */
    SessionManager(int maxInputsPerSec = 30, int maxShootsPerSec = 10)
        : _maxInputsPerSec(maxInputsPerSec), _maxShootsPerSec(maxShootsPerSec)
    {
    }

    /**
     * @brief Set callback invoked when a session is removed.
     */
    void setOnRemove(std::function<void(int)> cb)
    {
        _onRemove = std::move(cb);
    }
    /**
     * @brief Add a TCP session entry.
     */
    void addSession(int id, int tcpFd, const sockaddr_in &tcpAddr, long nowMs);
    /**
     * @brief Remove a session by player id.
     */
    void removeById(int id);
    /**
     * @brief Remove a session by TCP file descriptor.
     */
    void removeByFd(int fd);
    /**
     * @brief Register the client's UDP address.
     */
    bool setUdpAddr(int id, const sockaddr_in &udpAddr);
    /**
     * @brief Lookup a session by player id.
     */
    std::optional<Session> getSession(int id) const;
    /**
     * @brief Update last PONG time for heartbeat checks.
     */
    void updatePong(int id, long nowMs);
    /**
     * @brief Store the lobby code for a player.
     */
    void setLobbyCode(int id, const std::string &code);
    /**
     * @brief Fetch the lobby code for a player.
     */
    std::optional<std::string> getLobbyCode(int id) const;
    /**
     * @brief Store the player's pseudo.
     */
    void setPseudo(int id, const std::string &pseudo);
    /**
     * @brief Fetch the player's pseudo.
     */
    std::optional<std::string> getPseudo(int id) const;

    /**
     * @brief Rate-limit input packets per second.
     */
    bool allowInput(int id, long nowMs);
    /**
     * @brief Rate-limit shoot packets per second.
     */
    bool allowShoot(int id, long nowMs);

  private:
    void resetCounter(long nowMs, long &lastMs, int &count, int limit);

    mutable std::mutex _mutex;
    std::unordered_map<int, Session> _sessions;
    std::function<void(int)> _onRemove;
    const int _maxInputsPerSec;
    const int _maxShootsPerSec;
};
