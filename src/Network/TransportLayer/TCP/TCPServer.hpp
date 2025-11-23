/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** TCPServer
*/

#pragma once

#include <array>
#include <vector>
#include <netinet/in.h>
#include "TCPSocket.hpp"

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
        };
        bool performHandshake(int clientFd, int playerId);
        void acceptNewClient();
        void processClientData(Client &client);
        void sendPingToAll();
        void checkHeartbeat();
        long getCurrentTime();

    private:
        Network::TransportLayer::TCPSocket _serverSocket;
        std::array<Client, MAX_CLIENT> _clients;
        int _nextId = 1;
};