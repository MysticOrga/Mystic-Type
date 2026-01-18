/*
** EPITECH PROJECT, 2026
** Mystic-Type
** File description:
** UnixSocket
*/

#ifndef UNIXSOCKET_HPP_
#define UNIXSOCKET_HPP_
#include "../ASocket.hpp"
#include <iostream>
#ifdef _WIN32
    #define SOCK_TYPE SOCK_STREAM
#else
    #define SOCK_TYPE SOCK_DGRAM
#endif
#include <string>

namespace Network::TransportLayer
{
class UnixSocket : public ASocket
{
  public:
    /**
     * @brief Construct a new Unix Socket object
     *
     */
    UnixSocket() : ASocket(AF_UNIX, SOCK_TYPE, 0)
    {
        std::cout << "Je suis ici" << std::endl;
    };

    /**
     * @brief Construct a new Unix Socket object
     *
     * @param path
     */
    UnixSocket(const std::string &path);

    /**
     * @brief Destroy the Unix Socket object
     *
     */
    inline ~UnixSocket()
    {
        std::cout << "unlink: " << _addr.sun_path << std::endl; 
        ::unlink(_addr.sun_path);
    };

    bool setPath(const std::string &path);

    /**
     * @brief Check if socket is linked to an unix socket
     *
     * @return true linked
     * @return false not linked
     */
    inline bool isLink(void) const
    {
        return _linked;
    }

    /**
     * @brief unlink socket path
     *
     * @return true succes
     * @return false error
     */
    inline bool unink(void)
    {
        if (_linked)
        {
            std::cout << "unlink: " << _addr.sun_path << std::endl;
            return ::unlink(_addr.sun_path) == 0 ? true : false;
        }
        return false;
    }

    /**
     * @brief  Bind Unix socket to file specified
     *
     * @return true bind succesfull
     * @return false error happend
     */
    bool bind();

    /**
     * @brief Connect to a Unix socket
     *
     * @return true connection success
     * @return false error happen
     */
    bool connect();

  protected:
  private:
    sockaddr_un _addr;
    bool _linked;
};
} // namespace Network::TransportLayer

#endif /* !UNIXSOCKET_HPP_ */