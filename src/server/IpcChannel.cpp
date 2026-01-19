/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** IPC channel helper (UNIX domain socket)
*/

#include "IpcChannel.hpp"

#include <cstring>
#include <iostream>
namespace
{
constexpr std::size_t MAX_MSG = 1024;
}

IpcChannel::~IpcChannel()
{
    close();
}

bool IpcChannel::bindServer(void)
{
    if (_sockfd != INVALID_SOCKET_FD)
        CLOSE(_sockfd);

    _sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (_sockfd == INVALID_SOCKET_FD)
        return false;

    memset(&_addr, 0, sizeof(_addr));
    _addr.sin_family = AF_INET;
    _addr.sin_port = htons(0);
    _addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (::bind(_sockfd, reinterpret_cast<sockaddr *>(&_addr), sizeof(_addr)) == -1)
    {
        std::cerr << "Bind failed" << std::endl;
        CLOSE(_sockfd);
        return false;
    }

    socklen_t addrLen = sizeof(_addr);
    if (getsockname(_sockfd, reinterpret_cast<sockaddr *>(&_addr), &addrLen) == -1)
    {
        std::cerr << "getsockname failed" << std::endl;
        CLOSE(_sockfd);
        return false;
    }

    _isServer = true;
    return true;
}

bool IpcChannel::connectClient(const std::string &port)
{
    if (_sockfd != INVALID_SOCKET_FD)
        CLOSE(_sockfd);

    _sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (_sockfd == INVALID_SOCKET_FD)
        return false;

    memset(&_addr, 0, sizeof(_addr));
    _addr.sin_family = AF_INET;
    _addr.sin_port = htons(std::stoi(port));
    _addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    _isServer = false;
    return true;
}

bool IpcChannel::send(const std::string &msg)
{
    if (_sockfd == INVALID_SOCKET_FD)
        return false;
    if (msg.size() > MAX_MSG)
        return false;
    ssize_t sent =
        ::sendto(_sockfd, msg.c_str(), msg.size(), 0, reinterpret_cast<const sockaddr *>(&_addr), sizeof(_addr));
    return sent == static_cast<ssize_t>(msg.size());
}

std::optional<std::string> IpcChannel::recv(int timeoutMs)
{
    fd_set rfds = {0};
    timeval tv = {0};
    if (_sockfd == INVALID_SOCKET_FD)
        return std::nullopt;

    FD_ZERO(&rfds);
    FD_SET(_sockfd, &rfds);
    tv.tv_usec = timeoutMs;
    tv.tv_sec = 0;
    int ret = select(_sockfd + 1, &rfds, NULL, NULL, &tv);
    if (ret <= 0)
        return std::nullopt;

    char buf[MAX_MSG + 1]{};
    ssize_t n = ::recv(_sockfd, buf, MAX_MSG, 0);
    if (n <= 0)
        return std::nullopt;
    return std::string(buf, buf + n);
}

void IpcChannel::close()
{
    if (_sockfd != -INVALID_SOCKET_FD)
    {
        CLOSE(_sockfd);
    }
    _isServer = false;
    memset(&_addr, 0, sizeof(_addr));
}
