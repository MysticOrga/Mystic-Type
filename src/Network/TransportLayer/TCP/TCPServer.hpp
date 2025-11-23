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
#include <string_view>
#include <netinet/in.h>
#include <sys/select.h>
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
        bool waitForReadable(int fd, int timeoutSec, int timeoutUsec = 0);
        bool sendMessage(int fd, std::string_view message);
        bool receiveMessage(int fd, std::string &message);
        void closeFd(int &fd);
        void resetClient(Client &client);
        int pollSockets(fd_set &readfds, int maxFd, struct timeval &timeout);

    private:
        Network::TransportLayer::TCPSocket _serverSocket;
        std::array<Client, MAX_CLIENT> _clients;
        int _nextId = 1;
};
