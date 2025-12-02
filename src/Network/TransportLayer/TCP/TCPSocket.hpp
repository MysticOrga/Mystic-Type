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
    /**
     * @brief TCP socket wrapper providing connect/accept helpers.
     *
     * Inherits the basic file descriptor management from ASocket and adds
     * convenience methods for client connections and server listen/accept.
     */
    class TCPSocket : public ASocket
    {
    public:
        /**
         * @brief Create a TCP socket (AF_INET / SOCK_STREAM).
         */
        TCPSocket() : ASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP) {}

        ~TCPSocket() = default;

        /**
         * @brief Connect to a remote TCP endpoint.
         *
         * @param ip Destination IPv4 address (string form).
         * @param port Destination port.
         * @return true on success, false otherwise.
         */
        bool connectTo(const std::string &ip, std::uint16_t port);

        /**
         * @brief Bind to an address and start listening.
         *
         * @param port Local port to bind.
         * @param address Local IPv4 address (default: INADDR_ANY).
         * @param backlog listen backlog size.
         * @return true on success, false otherwise.
         */
        bool bindAndListen(std::uint16_t port, std::uint32_t address = INADDR_ANY, int backlog = 4);

        /**
         * @brief Accept an incoming client connection.
         *
         * @param clientAddr Filled with the connecting client's address.
         * @return int Accepted client socket fd, or -1 on error.
         */
        int acceptClient(struct sockaddr_in &clientAddr);
    };
}
