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
#include <unordered_map>
#include <netinet/in.h>
#include <sys/select.h>
#include <memory>
#include "TCPSocket.hpp"
#include "../Packet.hpp"
#include "../../SessionManager.hpp"
#include "server/ChildProcessManager.hpp"
#include "server/IpcChannel.hpp"

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
        TCPServer(uint16_t port, SessionManager &sessions, ChildProcessManager *childMgr = nullptr);

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
            int fd = -1;
            sockaddr_in addr{};
            bool handshakeDone = false;
            long lastPongTime = 0;
            long handshakeStart = 0;
            long long lastPingSentMs = 0;
            int pingMs = 0;
            std::string lobbyCode;
            std::string pseudo;

            uint8_t posX = 0;
            uint8_t posY = 0;
            uint8_t hp = 0;
            std::vector<uint8_t> recvBuffer;
        };

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
        long long getCurrentTimeMs();

        /**
         * @brief Block until a file descriptor is readable or times out.
         */
        bool waitForReadable(int fd, int timeoutSec, int timeoutUsec = 0);

        /**
         * @brief Close and invalidate a file descriptor.
         */
        void closeFd(int &fd);

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
        bool sendPacket(int fd, const Packet &packet);

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
        RecvResult receivePacket(int fd, Packet &packet, std::vector<uint8_t> &buffer);

        /**
         * @brief Build a packet carrying a string payload.
         */
        Packet makeStringPacket(PacketType type, const std::string &payload);

        /**
         * @brief Build a packet carrying a simple integer id.
         */
        Packet makeIdPacket(PacketType type, int value);
        Packet makeLobbyPacket(PacketType type, const std::string &payload);

        /**
         * @brief Write raw bytes to a socket fd.
         */
        ssize_t writeFd(int fd, const uint8_t *data, std::size_t size);

        /**
         * @brief Read raw bytes from a socket fd.
         */
        ssize_t readFd(int fd, uint8_t *data, std::size_t size);

        /**
         * @brief Wrapper around select for testability.
         */
        int selectFdSet(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);

        /**
         * @brief Close a raw fd without resetting the caller's variable.
         */
        int closeFdRaw(int fd);

        /**
         * @brief Build the packet containing the list of connected players.
         */
        Packet buildPlayerListPacket(const std::string &lobbyCode) const;

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
        bool writeAll(int fd, const uint8_t *data, std::size_t size);

        /**
         * @brief Assign a client to a lobby (public or private).
         */
        bool assignLobby(Client &client, const std::string &code, bool createIfMissing, bool isPublic, bool allowFull = false);

        /**
         * @brief Remove a client from its current lobby.
         */
        void removeFromLobby(const Client &client);

        /**
         * @brief Resend the player list to all members of a lobby.
         */
        void refreshLobby(const std::string &code);

        /**
         * @brief Auto-match the client into a public lobby.
         */
        std::string autoAssignPublic(Client &client);

        /**
         * @brief Generate a random lobby code (6 chars alnum).
         */
        std::string generateLobbyCode();

        /**
         * @brief Handle lobby-related packets for a client.
         */
        void handleLobbyPacket(Client &client, const Packet &packet);
        void processIpcMessages();
        void broadcastToLobby(const std::string &lobbyCode, const Packet &packet);

    private:
        Network::TransportLayer::TCPSocket _serverSocket;
        std::array<Client, MAX_CLIENT> _clients;
        int _nextId = 1;
        SessionManager &_sessions;
        struct LobbyInfo {
            bool isPublic = false;
            std::vector<int> players;
            uint16_t udpPort = 0;
            std::string ipcPath;
            std::unique_ptr<IpcChannel> ipc;
        };
        std::unordered_map<std::string, LobbyInfo> _lobbies;
        std::unordered_map<int, std::string> _clientLobby;
        ChildProcessManager *_childMgr = nullptr; // optional, not wired yet

        uint16_t allocatePort();
        void ensureLobbyProcess(const std::string &code, bool isPublic);
};
