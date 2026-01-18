/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** implement socket interface
*/

#ifndef ISOCKET_HPP_
#define ISOCKET_HPP_
#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #define NOGDI
    #define NOUSER
    // Windows includes
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <cstdint>
    #include <cstddef>
    #include <afunix.h>
    #pragma comment(lib, "ws2_32.lib")
    
    // Windows macros
    #define CLOSE(s) closesocket(s)
    #define SOCKET_ERROR_CODE WSAGetLastError()
    typedef SOCKET socket_t;
    #define INVALID_SOCKET_FD INVALID_SOCKET
    using ssize_t = std::ptrdiff_t;
    
#else
    // Linux includes
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <errno.h>
    #include <sys/select.h>
    #include <cstdint>
    #include <sys/un.h>
    #include <netinet/tcp.h>
    
    // Linux macros
    #define CLOSE(s) close(s)
    #define SOCKET_ERROR_CODE errno
    typedef int socket_t;
    #define INVALID_SOCKET_FD -1
    
#endif

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
        IOERROR // an error occurred
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
        inline virtual socket_t getSocketFd() const = 0;

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
        virtual ssize_t readByte(char *buffer, std::size_t size) = 0;

        /**
         * @brief Write bytes to the socket
         *
         * @param data Data to write
         * @param size Number of bytes to write
         * @return ssize_t Number of bytes written, or -1 on error
         */
        virtual ssize_t writeByte(const char *data, std::size_t size) = 0;

        /**
         * @brief Check the socket state for read, write, or exception
         *
         * @param timeoutSec Timeout in seconds
         * @param timeoutUsec Timeout in microseconds
         * @return IOState The state of the socket
         */
        virtual IOState sockState(int timeoutSec, int timeoutUsec) = 0;

    protected:
    private:
    };
}

#endif /* !ISOCKET_HPP_ */
