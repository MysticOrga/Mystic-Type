/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** IPC channel helper (UNIX domain socket)
*/

#include "IpcChannel.hpp"

#include <cstring>
#include <iostream>
#include <poll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <iostream>
namespace
{
constexpr std::size_t MAX_MSG = 1024;
}

IpcChannel::~IpcChannel()
{
    close();
}

bool IpcChannel::bindServer(const std::string &path)
{
    std::cout << "Bind this paths" << path << std::endl;
    _socket.setPath(path);
    if (_socket.bind() == false)
    {
        _socket.closeSocket();
        _socket.unink();
        return false;
    }
    _isServer = true;
    _path = path;
    return true;
}

bool IpcChannel::connectClient(const std::string &path)
{
    _socket.setPath(path);
    if (!_socket.connect())
    {
        perror("CONNECT: ");
        _socket.closeSocket();
        _socket.unink();
        return false;
    }
    _isServer = false;
    _path.clear();
    return true;
}

bool IpcChannel::send(const std::string &msg)
{
    if (_socket.getSocketFd() == INVALID_SOCKET_FD)
        return false;
    if (msg.size() > MAX_MSG)
        return false;
    ssize_t sent = ::send(_socket.getSocketFd(), msg.data(), msg.size(), 0);
    return sent == static_cast<ssize_t>(msg.size());
}

std::optional<std::string> IpcChannel::recv(int timeoutMs)
{
    if (_socket.getSocketFd() == INVALID_SOCKET_FD)
        return std::nullopt;

    struct pollfd pfd{};
    pfd.fd = _socket.getSocketFd();
    pfd.events = POLLIN;
    int ret = ::poll(&pfd, 1, timeoutMs);
    if (ret <= 0)
        return std::nullopt;

    char buf[MAX_MSG + 1]{};
    ssize_t n = ::recv(_socket.getSocketFd(), buf, MAX_MSG, 0);
    if (n <= 0)
        return std::nullopt;
    return std::string(buf, buf + n);
}

void IpcChannel::close()
{
    if (_socket.getSocketFd() != -INVALID_SOCKET_FD)
    {
        _socket.closeSocket();
    }
    if (_isServer && !_path.empty())
    {
        _socket.unink();
    }
    _isServer = false;
    _path.clear();
}
