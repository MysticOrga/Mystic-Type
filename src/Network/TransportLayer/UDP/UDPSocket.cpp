/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** UDP Socket
*/

#include "UDPSocket.hpp"

namespace Network::TransportLayer
{
    ssize_t UDPSocket::writeByte(const void *data, std::size_t size)
    {
        return sendto(_socketFd, data, size, 0,
                        reinterpret_cast<struct sockaddr *>(&_senderAddr),
                        sizeof(_senderAddr));
    }

    ssize_t UDPSocket::writeByte(const void *data, std::size_t size, const struct sockaddr_in &destAddr)
    {
        return sendto(_socketFd, data, size, 0,
                        reinterpret_cast<struct sockaddr *>(const_cast<struct sockaddr_in *>(&destAddr)),
                        sizeof(destAddr));
    }

    ssize_t UDPSocket::readByte(void *buffer, std::size_t size)
    {
        socklen_t addrLen = sizeof(_senderAddr);
        return recvfrom(_socketFd, buffer, size, 0,
                        reinterpret_cast<struct sockaddr *>(&_senderAddr),
                        &addrLen);
    }
}
