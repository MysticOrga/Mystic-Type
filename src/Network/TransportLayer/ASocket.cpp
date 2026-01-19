/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** abstract class for socket class
*/

#include "ASocket.hpp"
#include <iostream>
#include <stdexcept>
// Implementation of ASocket methods
namespace Network::TransportLayer
{
ASocket::ASocket(int domain, int type, int protocol)
{
    _socketFd = socket(domain, type, protocol);
    if (_socketFd == INVALID_SOCKET_FD)
    {
        throw std::runtime_error("Failed to create socket");
    }
}

ASocket::~ASocket()
{
    closeSocket();
}

int ASocket::closeSocket()
{
    if (_socketFd != -INVALID_SOCKET_FD)
    {
        int result = CLOSE(_socketFd);
        _socketFd = INVALID_SOCKET_FD;
        return result;
    }
    return 0; // Socket already closed
}

IOState ASocket::sockState(int timeoutSec, int timeoutUsec)
{
    fd_set readfds, writefds, exceptfds;
    struct timeval timeout;

    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_ZERO(&exceptfds);
    FD_SET(_socketFd, &readfds);
    FD_SET(_socketFd, &writefds);
    FD_SET(_socketFd, &exceptfds);

    timeout.tv_sec = timeoutSec;
    timeout.tv_usec = timeoutUsec;

    int activity = select(_socketFd + 1, &readfds, &writefds, &exceptfds, &timeout);
    if (activity < 0)
    {
        return IOState::IOERROR;
    }
    if (activity != 0)
    {
        if (FD_ISSET(_socketFd, &readfds))
        {
            return IOState::READ_READY;
        }
        if (FD_ISSET(_socketFd, &exceptfds))
        {
            return IOState::EXCEPTION;
        }
        if (FD_ISSET(_socketFd, &writefds))
        {
            return IOState::WRITE_READY;
        }
        return IOState::TIMEOUT;
    }
    return IOState::TIMEOUT;
}

bool ASocket::bindSock(int domain, int port, uint32_t address)
{
    _addr.sin_family = domain;
    _addr.sin_port = htons(port);
    _addr.sin_addr.s_addr = htonl(address);

    return bind(_socketFd, reinterpret_cast<struct sockaddr *>(&_addr), sizeof(_addr)) == 0;
}

ssize_t ASocket::writeByte(const char *data, std::size_t size)
{
    if (data == nullptr || size == 0)
        return -1; // Invalid data or size
    if (sockState(0, 0) == IOState::WRITE_READY)
        return send(_socketFd, data, size, 0);
    return -1; // Socket not ready for writing
}

ssize_t ASocket::readByte(char *buffer, std::size_t size)
{
    if (buffer == nullptr || size == 0)
        return -1; // Invalid buffer or size
    if (sockState(0, 0) == IOState::READ_READY)
        return recv(_socketFd, buffer, size, 0);
    return -1; // Socket not ready for reading
}
} // namespace Network::TransportLayer
