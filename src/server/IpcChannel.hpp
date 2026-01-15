/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** IPC channel helper (UNIX domain socket)
*/

#pragma once

#include <string>
#include <optional>
#include "../Network/TransportLayer/UNIX/UnixSocket.hpp"
/**
 * @brief Small IPC helper based on a UNIX datagram socket (SOCK_DGRAM).
 *
 * Server mode: bind() on a path, then recv/send.
 * Client mode: connect() to the existing path, then recv/send.
 *
 * Messages are short ASCII strings (<= 1024 bytes); this is not a reliable stream
 * protocol but sufficient for READY/ALLOW/etc control signals.
 */
class IpcChannel {
public:
    IpcChannel() = default;
    ~IpcChannel();

    IpcChannel(const IpcChannel&) = delete;
    IpcChannel& operator=(const IpcChannel&) = delete;

    /**
     * @brief Create a socket and bind to a path (server side).
     * @return true on success.
     */
    bool bindServer(const std::string &path);

    /**
     * @brief Create a socket and connect to a path (client side).
     * @return true on success.
     */
    bool connectClient(const std::string &path);

    /**
     * @brief Send a text message (limited to 1024 bytes).
     */
    bool send(const std::string &msg);

    /**
     * @brief Receive a message with optional timeout (ms).
     * timeoutMs = 0 => non-blocking, <0 => blocking, >0 => poll with timeout.
     */
    std::optional<std::string> recv(int timeoutMs = 0);

    /**
     * @brief Close the socket and unlink the path when in server mode.
     */
    void close();
    int fd() const { return _socket.getSocketFd(); }

private:
    Network::TransportLayer::UnixSocket _socket;
    bool _isServer = false;
    std::string _path;
};
