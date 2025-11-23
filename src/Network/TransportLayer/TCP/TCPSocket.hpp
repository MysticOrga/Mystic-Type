/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** TCPSocket
*/

#pragma once
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <cstdint>
#include "../ASocket.hpp"

namespace Network::TransportLayer
{
    class TCPSocket : public ASocket
    {
    public:
        TCPSocket() : ASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP) {}

        ~TCPSocket() = default;
        bool connectTo(const std::string &ip, std::uint16_t port);
        bool bindAndListen(std::uint16_t port, std::uint32_t address = INADDR_ANY, int backlog = 4);
        int acceptClient(struct sockaddr_in &clientAddr);
    };
}
