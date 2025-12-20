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
#include <unordered_map>
#include <mutex>
#include <optional>
#include <functional>
#include <cstdint>
#include <vector>

struct Session {
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
};

/**
 * @brief Tracks client sessions across TCP and UDP, and enforces simple rate limits.
 */
class SessionManager {
public:
    SessionManager(int maxInputsPerSec = 30, int maxShootsPerSec = 10)
    : _maxInputsPerSec(maxInputsPerSec), _maxShootsPerSec(maxShootsPerSec) {}

    void setOnRemove(std::function<void(int)> cb) { _onRemove = std::move(cb); }
    void addSession(int id, int tcpFd, const sockaddr_in &tcpAddr, long nowMs);
    void removeById(int id);
    void removeByFd(int fd);
    bool setUdpAddr(int id, const sockaddr_in &udpAddr);
    std::optional<Session> getSession(int id) const;
    void updatePong(int id, long nowMs);

    bool allowInput(int id, long nowMs);
    bool allowShoot(int id, long nowMs);

private:
    void resetCounter(long nowMs, long &lastMs, int &count, int limit);

    mutable std::mutex _mutex;
    std::unordered_map<int, Session> _sessions;
    std::function<void(int)> _onRemove;
    const int _maxInputsPerSec;
    const int _maxShootsPerSec;
};
