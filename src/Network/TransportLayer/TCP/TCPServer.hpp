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

#define MAX_CLIENT 4
#define BUFFER_SIZE 1024

class TCPServer {
    public:
        TCPServer(uint16_t port);
        ~TCPServer();
        void run();

    private:
        struct Client
        {
            int id = 0;
            int fd = -1;
            sockaddr_in addr{};
            bool handshakeDone = false;
            long lastPongTime = 0;

            uint8_t posX = 0;
            uint8_t posY = 0;
        };
        bool performHandshake(int clientFd, int playerId);
        void acceptNewClient();
        void processClientData(Client &client);
        void sendPingToAll();
        void checkHeartbeat();
        long getCurrentTime();
        bool waitForReadable(int fd, int timeoutSec, int timeoutUsec = 0);
        void closeFd(int &fd);
        void resetClient(Client &client);
        int pollSockets(fd_set &readfds, int maxFd, struct timeval &timeout);
        bool sendPacket(int fd, const Packet &packet);
        bool receivePacket(int fd, Packet &packet);
        Packet makeStringPacket(PacketType type, const std::string &payload);
        Packet makeIdPacket(PacketType type, int value);
        ssize_t writeFd(int fd, const uint8_t *data, std::size_t size);
        ssize_t readFd(int fd, uint8_t *data, std::size_t size);
        int selectFdSet(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
        int closeFdRaw(int fd);
        Packet buildPlayerListPacket() const;
        void sendPlayerListToClient(const Client &client);
        void broadcastNewPlayer(const Client &newClient);

    private:
        Network::TransportLayer::TCPSocket _serverSocket;
        std::array<Client, MAX_CLIENT> _clients;
        int _nextId = 1;
};
