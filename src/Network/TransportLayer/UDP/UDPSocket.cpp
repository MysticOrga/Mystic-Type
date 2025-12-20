/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** UDP Socket
*/

#include "UDPSocket.hpp"

namespace Network::TransportLayer
{
    bool UDPSocket::bindTo(std::uint16_t port, std::uint32_t address)
    {
        return bindSock(AF_INET, port, address);
    }

    ssize_t UDPSocket::writeByte(const char *data, std::size_t size)
    {
        if (data == nullptr || size == 0)
            return -1; // Invalid data or size
        // if (sockState(0, 0) != IOState::WRITE_READY)
            // return -1; // Socket not ready for writing
        return sendto(_socketFd, data, size, 0,
                        reinterpret_cast<struct sockaddr *>(&_senderAddr),
                        sizeof(_senderAddr));
    }

    ssize_t UDPSocket::writeByte(const char *data, std::size_t size, const struct sockaddr_in &destAddr)
    {
        if (data == nullptr || size == 0)
            return -1; // Invalid data or size
        // if (sockState(0, 0) != IOState::WRITE_READY)
        //     return -1; // Socket not ready for writing
        return sendto(_socketFd, data, size, 0,
                        reinterpret_cast<struct sockaddr *>(const_cast<struct sockaddr_in *>(&destAddr)),
                        sizeof(destAddr));
    }

    ssize_t UDPSocket::readByte(char *buffer, std::size_t size)
    {
        socklen_t addrLen = sizeof(_senderAddr);
        if (buffer == nullptr || size == 0)
            return -1; // Invalid buffer or size
        if (sockState(0, 0) != IOState::READ_READY)
            return -1; // Socket not ready for reading
        return recvfrom(_socketFd, buffer, size, 0,
                        reinterpret_cast<struct sockaddr *>(&_senderAddr),
                        &addrLen);
    }
}
