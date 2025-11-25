/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** abstract class for socket class
*/

#include "ASocket.hpp"
#include <stdexcept>
#include <iostream>

// Implementation of ASocket methods
namespace Network::TransportLayer
{
    ASocket::ASocket(int domain, int type, int protocol)
    {
        _socketFd = socket(domain, type, protocol);
        if (_socketFd == -1)
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
        if (_socketFd != -1)
        {
            int result = close(_socketFd);
            _socketFd = -1;
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
            std::cout << "Eroor" <<std::endl;
            return IOState::ERROR;
        }
        if (activity != 0)
        {
            // std::cout << "Activiry" << std::endl;
            if (FD_ISSET(_socketFd, &readfds))
            {
                // std::cout << "Read ready" << std::endl;
                return IOState::READ_READY;
            }
            if (FD_ISSET(_socketFd, &exceptfds))
            {
                // std::cout << "Exception" << std::endl;
                return IOState::EXCEPTION;
            }
            if (FD_ISSET(_socketFd, &writefds))
            {
                // std::cout << "Write ready" << std::endl;
                return IOState::WRITE_READY;
            }
            // std::cout << "timeout" << std::endl;
            return IOState::TIMEOUT;
        }
        // std::cout << "timeout" << std::endl;
        return IOState::TIMEOUT;
    }

    bool ASocket::bindSock(int domain, int port, uint32_t address)
    {
        _addr.sin_family = domain;
        _addr.sin_port = htons(port);
        _addr.sin_addr.s_addr = htonl(address);

        return bind(_socketFd, reinterpret_cast<struct sockaddr *>(&_addr), sizeof(_addr)) == 0;
    }

    ssize_t ASocket::writeByte(const void *data, std::size_t size)
    {
            return write(_socketFd, data, size);
    }

    ssize_t ASocket::readByte(void *buffer, std::size_t size)
    {
            return read(_socketFd, buffer, size);
    }
}
