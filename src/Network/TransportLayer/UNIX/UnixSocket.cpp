/*
** EPITECH PROJECT, 2026
** Mystic-Type
** File description:
** UnixSocket
*/

#include "UnixSocket.hpp"
#include <iostream>
Network::TransportLayer::UnixSocket::UnixSocket(const std::string &path) : ASocket(AF_UNIX, SOCK_DGRAM, 0)
{
    _addr.sun_family = AF_UNIX;
    if (path.size() >= sizeof(_addr.sun_path))
        return;
    strncpy(_addr.sun_path, path.c_str(), path.size());
    _addr.sun_path[path.size()] = 0;
    _linked = true;
}

bool Network::TransportLayer::UnixSocket::setPath(const std::string &path)
{
    _addr.sun_family = AF_UNIX;
    if (path.size() >= sizeof(_addr.sun_path))
        return false;
    strncpy(_addr.sun_path, path.c_str(), path.size());
    _addr.sun_path[path.size()] = 0;
    std::cout << "Unix path: " << _addr.sun_path << std::endl;
    _linked = true;
    return true;
}

bool Network::TransportLayer::UnixSocket::bind()
{
    if (::bind(_socketFd, reinterpret_cast<sockaddr *>(&_addr), sizeof(_addr)) == -1)
    {
        perror("IPC: ");
        this->closeSocket();
        return false;
    }
    return true;
}

bool Network::TransportLayer::UnixSocket::connect()
{
    if (::connect(_socketFd, reinterpret_cast<sockaddr *>(&_addr), sizeof(_addr)) == -1)
    {
        this->closeSocket();
        return false;
    }
    return true;
}