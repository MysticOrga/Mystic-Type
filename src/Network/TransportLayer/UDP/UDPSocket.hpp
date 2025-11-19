/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** UDP socket
*/

#ifndef UDPSOCKET_HPP_
#define UDPSOCKET_HPP_
#include "../ASocket.hpp"

/**
 * @namespace Network::TransportLayer
 * @brief Namespace for network transport layer components
 */
namespace Network::TransportLayer
{
    /**
     * @brief UDP Socket class inheriting from ASocket
     *
     */
    class UDPSocket : public ASocket
    {
    public:
        /**
         * @brief Construct a new UDPSocket object
         *
         */
        UDPSocket() : ASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP) {};

        /**
         * @brief Destroy the UDPSocket object
         *
         */
        ~UDPSocket() = default;

        /**
         * @brief Write bytes to the UDP socket
         *
         * @param data Data to write
         * @param size Number of bytes to write
         * @return ssize_t Number of bytes written, or -1 on error
         */
        ssize_t writeByte(const void *data, std::size_t size) override;

        /**
         * @brief Write bytes to the UDP socket to a specific address
         *
         * @param data Data to write
         * @param size Number of bytes to write
         * @param destAddr Destination address to send data to
         * @return ssize_t Number of bytes written, or -1 on error
         */
        ssize_t writeByte(const void *data, std::size_t size, const struct sockaddr_in &destAddr);

        /**
         * @brief Read bytes from the UDP socket
         *
         * @param buffer Buffer to store the read data
         * @param size Number of bytes to read
         * @return ssize_t Number of bytes read, or -1 on error
         */
        ssize_t readByte(void *buffer, std::size_t size) override;

        /**
         * @brief Get the sender address of the last received packet
         *
         * @return struct sockaddr_in The sender's address
         */
        inline struct sockaddr_in getSenderAddr() const { return _senderAddr; }

    protected:
    private:
        struct sockaddr_in _senderAddr;
    };
}

#endif /* !UDPSOCKET_HPP_ */
