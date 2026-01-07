/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** IPC channel helper (UNIX domain socket)
*/

#include "IpcChannel.hpp"

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <poll.h>
#include <cstring>
#include <iostream>

namespace {
    constexpr std::size_t MAX_MSG = 1024;
}

IpcChannel::~IpcChannel()
{
    close();
}

bool IpcChannel::bindServer(const std::string &path)
{
    close();
    _fd = ::socket(AF_UNIX, SOCK_DGRAM, 0);
    if (_fd == -1)
        return false;

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    if (path.size() >= sizeof(addr.sun_path)) {
        std::cerr << "[IPC] path too long\n";
        ::close(_fd);
        _fd = -1;
        return false;
    }
    std::strncpy(addr.sun_path, path.c_str(), sizeof(addr.sun_path) - 1);
    ::unlink(addr.sun_path); // clean previous
    if (::bind(_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1) {
        ::close(_fd);
        _fd = -1;
        return false;
    }
    _isServer = true;
    _path = path;
    return true;
}

bool IpcChannel::connectClient(const std::string &path)
{
    close();
    _fd = ::socket(AF_UNIX, SOCK_DGRAM, 0);
    if (_fd == -1)
        return false;

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    if (path.size() >= sizeof(addr.sun_path)) {
        std::cerr << "[IPC] path too long\n";
        ::close(_fd);
        _fd = -1;
        return false;
    }
    std::strncpy(addr.sun_path, path.c_str(), sizeof(addr.sun_path) - 1);
    if (::connect(_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1) {
        ::close(_fd);
        _fd = -1;
        return false;
    }
    _isServer = false;
    _path.clear();
    return true;
}

bool IpcChannel::send(const std::string &msg)
{
    if (_fd == -1)
        return false;
    if (msg.size() > MAX_MSG)
        return false;
    ssize_t sent = ::send(_fd, msg.data(), msg.size(), 0);
    return sent == static_cast<ssize_t>(msg.size());
}

std::optional<std::string> IpcChannel::recv(int timeoutMs)
{
    if (_fd == -1)
        return std::nullopt;

    struct pollfd pfd{};
    pfd.fd = _fd;
    pfd.events = POLLIN;
    int ret = ::poll(&pfd, 1, timeoutMs);
    if (ret <= 0)
        return std::nullopt;

    char buf[MAX_MSG + 1]{};
    ssize_t n = ::recv(_fd, buf, MAX_MSG, 0);
    if (n <= 0)
        return std::nullopt;
    return std::string(buf, buf + n);
}

void IpcChannel::close()
{
    if (_fd != -1) {
        ::close(_fd);
        _fd = -1;
    }
    if (_isServer && !_path.empty()) {
        ::unlink(_path.c_str());
    }
    _isServer = false;
    _path.clear();
}
