/*
** EPITECH PROJECT, 2025
** R-type
** File description:
** UDPSocket
*/

#ifndef UDPSocket_HPP_
#define UDPSocket_HPP_
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>

namespace RNetwork
{
    class UDPSocket
    {
    public:
        UDPSocket() : _sockfd(0) {};
        /**
         * @brief Destroy the UDPSocket object
         */
        ~UDPSocket() = default;

        inline void createSock(int domain, int type, int proto)
        {
            _sockfd = socket(domain, type, proto);
        }

        void setAddr(int domain, uint16_t port, in_addr_t addr);

        /**
         * @brief Get the Socket object
         *
         * @return the fd of the socket
         */

        inline int getSocket() const
        {
            return _sockfd;
        }

        inline sockaddr_in getAddr() const
        {
            return _addr;
        }
        /**
         * @brief
         *
         * @param addr address to send the message
         * @param buffer the data to send
         * @return ssize_t number of bytes send, -1 in error case
         */
        inline ssize_t send(struct sockaddr *addr, std::string buffer) const
        {
            return sendto(_sockfd, buffer.c_str(), buffer.length(), 0, addr, sizeof(addr));
        }

        inline bool bindSock()
        {
            return bind(_sockfd, (struct sockaddr*)&_addr, sizeof(_addr)) == -1 ? false: true;
        }
        /**
         * @brief
         *
         * @param addr fill the address with the sender address
         * @param buffer put data into the buffer
         * @return ssize_t number of bytes receive, -1 in error case
         */
        inline ssize_t recv(struct sockaddr *addr, char *buffer, size_t len) const
        {
            socklen_t slen = 0;
            return recvfrom(_sockfd, buffer, len, 0, addr, &slen);
        }

    protected:
    private:
        int _sockfd;
        sockaddr_in _addr;
    };
}

#endif /* !UDPSocket_HPP_ */
