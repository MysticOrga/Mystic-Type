/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** class abstraction for socket
*/

#ifndef ASOCKET_HPP_
#define ASOCKET_HPP_
#include "ISocket.hpp"
#include <sys/socket.h>
#include <arpa/inet.h>

namespace Network::TransportLayer
{
    class ASocket : public ISocket
    {
    public:
        /**
         * @brief Construct a new ASocket object
         *
         * @param domain The communication domain (e.g., AF_INET)
         * @param type The communication type (e.g., SOCK_STREAM)
         * @param protocol The protocol to be used (e.g., IPPROTO_TCP)
         */
        ASocket(int domain, int type, int protocol);

        /**
         * @brief Destroy the ASocket object
         *
         */
        ~ASocket();

        /**
         * @brief Get the underlying socket file descriptor
         *
         * @return int The socket file descriptor
         */
        inline int getSocketFd() const override
        {
            return _socketFd;
        }

        /**
         * @brief Close the socket
         *
         */
        int closeSocket() override;

        /**
         * @brief Bind the socket to a specific address and port
         *
         * @param domain The communication domain (e.g., AF_INET)
         * @param port The port number to bind to
         * @param address The IP address to bind to (in network byte order)
         * @return true if binding is successful, false otherwise
         */
        bool bindSock(int domain, int port, uint32_t address);

        /**
         * @brief Read bytes from the socket
         *
         * @param buffer Buffer to store the read data
         * @param size Number of bytes to read
         * @return ssize_t Number of bytes read, or -1 on error
         */
        virtual ssize_t readByte(void *buffer, std::size_t size) override;

        /**
         * @brief Write bytes to the socket
         *
         * @param data Data to write
         * @param size Number of bytes to write
         * @return ssize_t Number of bytes written, or -1 on error
         */
        virtual ssize_t writeByte(const void *data, std::size_t size) override;

        /**
         * @brief Check the socket state for read, write, or exception
         *
         * @param timeoutSec Timeout in seconds
         * @param timeoutUsec Timeout in microseconds
         * @return IOState The state of the socket
         */
        IOState sockState(int timeoutSec, int timeoutUsec) override;

    protected:
        int _socketFd;              // native socket descriptor
        struct sockaddr_in _addr;   // bound address/port for the socket
    private:
    };
}

#endif /* !ASOCKET_HPP_ */
