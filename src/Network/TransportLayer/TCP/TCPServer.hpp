/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** TCPServer
*/

#pragma once

#include <array>
#include <vector>
#include <string>
#include <netinet/in.h>
#include <sys/select.h>
#include "TCPSocket.hpp"
#include "../Packet.hpp"
#include "../../SessionManager.hpp"

#include <vector>

#define MAX_CLIENT 4
#define BUFFER_SIZE 1024

/**
 * @brief Simple TCP server handling client connections and lobby state.
 *
 * The server accepts new TCP clients, performs a handshake, keeps a small list
 * of connected players, and forwards heartbeat and player list packets to them.
 */
class TCPServer {
    public:
        /**
         * @brief Construct a new TCPServer bound to the given port.
         *
         * @param port TCP port to listen on.
         */
        TCPServer(uint16_t port, SessionManager &sessions);

        /**
         * @brief Destroy the TCPServer, closing the listening socket.
         */
        ~TCPServer();

        /**
         * @brief Run the main accept/receive loop (blocking).
         */
        void run();

    private:
        /**
         * @brief Lightweight representation of a connected client.
         */
        struct Client
        {
            int id = 0;
            socket_t fd = INVALID_SOCKET_FD;
            sockaddr_in addr{};
            bool handshakeDone = false;
            long lastPongTime = 0;

            uint8_t posX = 0;
            uint8_t posY = 0;
            std::vector<uint8_t> recvBuffer;
        };

        /**
         * @brief Complete initial handshake and assign an id to a new client.
         */
        bool performHandshake(Client &client);

        /**
         * @brief Accept incoming connection and register a new client slot.
         */
        void acceptNewClient();

        /**
         * @brief Read and handle data for a single client.
         */
        void processClientData(Client &client);

        /**
         * @brief Send a ping packet to every connected client.
         */
        void sendPingToAll();

        /**
         * @brief Disconnect clients whose heartbeat timed out.
         */
        void checkHeartbeat();

        /**
         * @brief Get current time in milliseconds.
         */
        long getCurrentTime();

        /**
         * @brief Block until a file descriptor is readable or times out.
         */
        bool waitForReadable(socket_t fd, int timeoutSec, int timeoutUsec = 0);

        /**
         * @brief Close and invalidate a file descriptor.
         */
        void closeFd(socket_t &fd);

        /**
         * @brief Reset client fields and close its socket.
         */
        void resetClient(Client &client);

        /**
         * @brief Poll all sockets using select().
         *
         * @return Number of ready descriptors or -1 on error.
         */
        int pollSockets(fd_set &readfds, int maxFd, struct timeval &timeout);

        /**
         * @brief Send a packet to a specific client fd.
         */
        bool sendPacket(socket_t fd, const Packet &packet);

        /**
         * @brief Result of a receive attempt.
         */
        enum class RecvResult { Disconnected, Incomplete, Ok };

        /**
         * @brief Receive and parse a packet from a client socket.
         *
         * @param fd Client socket.
         * @param packet Output parsed packet.
         * @param buffer Receive buffer for reassembly.
         */
        RecvResult receivePacket(socket_t fd, Packet &packet, std::vector<uint8_t> &buffer);

        /**
         * @brief Build a packet carrying a string payload.
         */
        Packet makeStringPacket(PacketType type, const std::string &payload);

        /**
         * @brief Build a packet carrying a simple integer id.
         */
        Packet makeIdPacket(PacketType type, int value);

        /**
         * @brief Write raw bytes to a socket fd.
         */
        ssize_t writeFd(socket_t fd, const uint8_t *data, std::size_t size);

        /**
         * @brief Read raw bytes from a socket fd.
         */
        ssize_t readFd(socket_t fd, uint8_t *data, std::size_t size);

        /**
         * @brief Wrapper around select for testability.
         */
        int selectFdSet(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);

        /**
         * @brief Close a raw fd without resetting the caller's variable.
         */
        int closeFdRaw(socket_t fd);

        /**
         * @brief Build the packet containing the list of connected players.
         */
        Packet buildPlayerListPacket() const;

        /**
         * @brief Send the current player list to a single client.
         */
        void sendPlayerListToClient(const Client &client);

        /**
         * @brief Notify all clients of a newly connected player.
         */
        void broadcastNewPlayer(const Client &newClient);

        /**
         * @brief Ensure the full buffer is written to the socket.
         */
        bool writeAll(socket_t fd, const uint8_t *data, std::size_t size);

    private:
        Network::TransportLayer::TCPSocket _serverSocket;
        std::array<Client, MAX_CLIENT> _clients;
        int _nextId = 1;
        SessionManager &_sessions;
};
