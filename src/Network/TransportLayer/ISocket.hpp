/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** implement socket interface
*/

#ifndef ISOCKET_HPP_
#define ISOCKET_HPP_
#include <unistd.h>
#include <cstddef>

namespace Network::TransportLayer
{
    /**
     * @enum IOState
     * @brief Represents the state of I/O operations on a socket.
     */
    enum IOState
    {
        READ_READY, // the socket is ready for reading
        WRITE_READY, // the socket is ready for writing
        EXCEPTION, // an exception occurred on the socket
        TIMEOUT, // the operation timed out
        ERROR // an error occurred
    };

    /**
     * @class ISocket
     * @brief Interface for socket operations.
     */
    class ISocket
    {
    public:
        /**
         * @brief Destroy the ISocket object
         *
         */
        virtual ~ISocket() = default;

        /**
         * @brief Get the underlying socket file descriptor
         *
         * @return int The socket file descriptor
         */
        inline virtual int getSocketFd() const = 0;

        /**
         * @brief Close the socket
         *
         */
        inline virtual int closeSocket() = 0;

        /**
         * @brief Read bytes from the socket
         *
         * @param buffer Buffer to store the read data
         * @param size Number of bytes to read
         * @return ssize_t Number of bytes read, or -1 on error
         */
        virtual ssize_t readByte(void *buffer, std::size_t size) = 0;

        /**
         * @brief Write bytes to the socket
         *
         * @param data Data to write
         * @param size Number of bytes to write
         * @return ssize_t Number of bytes written, or -1 on error
         */
        virtual ssize_t writeByte(const void *data, std::size_t size) = 0;

        /**
         * @brief Check the socket state for read, write, or exception
         *
         * @param timeoutSec Timeout in seconds
         * @param timeoutUsec Timeout in microseconds
         * @return IOState The state of the socket
         */
        virtual IOState SockState(int timeoutSec, int timeoutUsec) = 0;

    protected:
    private:
    };
}

#endif /* !ISOCKET_HPP_ */
