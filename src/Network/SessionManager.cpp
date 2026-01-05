/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** Session manager shared between TCP and UDP servers
*/

#include "SessionManager.hpp"

void SessionManager::addSession(int id, int tcpFd, const sockaddr_in &tcpAddr, long nowMs)
{
    std::lock_guard<std::mutex> lock(_mutex);
    Session s;
    s.id = id;
    s.tcpFd = tcpFd;
    s.tcpAddr = tcpAddr;
    s.lastPong = nowMs;
    s.lobbyCode.clear();
    _sessions[id] = s;
}

void SessionManager::removeById(int id)
{
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = _sessions.find(id);
    if (it != _sessions.end()) {
        _sessions.erase(it);
        if (_onRemove) {
            _onRemove(id);
        }
    }
}

void SessionManager::removeByFd(int fd)
{
    std::lock_guard<std::mutex> lock(_mutex);
    for (auto it = _sessions.begin(); it != _sessions.end(); ) {
        if (it->second.tcpFd == fd) {
            int id = it->second.id;
            it = _sessions.erase(it);
            if (_onRemove) {
                _onRemove(id);
            }
        } else {
            ++it;
        }
    }
}

bool SessionManager::setUdpAddr(int id, const sockaddr_in &udpAddr)
{
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = _sessions.find(id);
    if (it == _sessions.end())
        return false;
    it->second.udpAddr = udpAddr;
    it->second.hasUdp = true;
    return true;
}

std::optional<Session> SessionManager::getSession(int id) const
{
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = _sessions.find(id);
    if (it == _sessions.end())
        return std::nullopt;
    return it->second;
}

void SessionManager::updatePong(int id, long nowMs)
{
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = _sessions.find(id);
    if (it == _sessions.end())
        return;
    it->second.lastPong = nowMs;
}

void SessionManager::setLobbyCode(int id, const std::string &code)
{
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = _sessions.find(id);
    if (it == _sessions.end())
        return;
    it->second.lobbyCode = code;
}

std::optional<std::string> SessionManager::getLobbyCode(int id) const
{
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = _sessions.find(id);
    if (it == _sessions.end())
        return std::nullopt;
    return it->second.lobbyCode;
}

void SessionManager::resetCounter(long nowMs, long &lastMs, int &count, int limit)
{
    if (nowMs - lastMs >= 1000) {
        lastMs = nowMs;
        count = 0;
    }
    if (count < limit)
        count++;
}

bool SessionManager::allowInput(int id, long nowMs)
{
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = _sessions.find(id);
    if (it == _sessions.end())
        return false;
    resetCounter(nowMs, it->second.lastInputMs, it->second.inputCount, _maxInputsPerSec);
    return it->second.inputCount <= _maxInputsPerSec;
}

bool SessionManager::allowShoot(int id, long nowMs)
{
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = _sessions.find(id);
    if (it == _sessions.end())
        return false;
    resetCounter(nowMs, it->second.lastShootMs, it->second.shootCount, _maxShootsPerSec);
    return it->second.shootCount <= _maxShootsPerSec;
}
