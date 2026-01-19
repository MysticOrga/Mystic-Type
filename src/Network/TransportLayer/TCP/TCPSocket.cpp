/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** TCPSocket
*/

#include "TCPSocket.hpp"

namespace Network::TransportLayer
{
bool TCPSocket::connectTo(const std::string &ip, std::uint16_t port)
{
    struct sockaddr_in dest
    {
    };
    dest.sin_family = AF_INET;
    dest.sin_port = htons(port);
    if (inet_pton(AF_INET, ip.c_str(), &dest.sin_addr) <= 0)
    {
        return false;
    }
    if (::connect(_socketFd, reinterpret_cast<struct sockaddr *>(&dest), sizeof(dest)) == -1)
    {
        return false;
    }
    return true;
}

bool TCPSocket::bindAndListen(std::uint16_t port, std::uint32_t address, int backlog)
{
    if (!bindSock(AF_INET, port, address))
        return false;
    int opt = 1;
    if (setsockopt(_socketFd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(&opt), sizeof(opt)) < 0)
    {
        return false;
    }
    if (::listen(_socketFd, backlog) == -1)
        return false;

    return true;
}

int TCPSocket::acceptClient(struct sockaddr_in &clientAddr)
{
    socklen_t len = sizeof(clientAddr);
    int clientFd = ::accept(_socketFd, reinterpret_cast<struct sockaddr *>(&clientAddr), &len);
    return clientFd;
}
} // namespace Network::TransportLayer