/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** TCPServer
*/

#include "TCPServer.hpp"
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <sys/select.h>

TCPServer::TCPServer(uint16_t port)
{
    if (!_serverSocket.bindAndListen(port, INADDR_ANY, MAX_CLIENT)) {
        throw std::runtime_error("Failed to start TCP server");
    }

    std::cout << "[SERVER] TCPServer started on port " << port << std::endl;
}

TCPServer::~TCPServer()
{
    for (auto &c : _clients) {
        if (c.fd != -1) {
            ::close(c.fd);
        }
    }
    _serverSocket.closeSocket();
}

bool TCPServer::performHandshake(int clientFd, int playerId)
{
    const char *serverHello = "R-Type Server\n";
    ::write(clientFd, serverHello, strlen(serverHello));

    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(clientFd, &rfds);

    struct timeval timeout{3, 0};

    int ret = select(clientFd + 1, &rfds, nullptr, nullptr, &timeout);
    if (ret <= 0) {
        const char *msg = "REFUSED TIMEOUT\n";
        ::write(clientFd, msg, strlen(msg));
        return false;
    }

    char buffer[BUFFER_SIZE]{};
    ssize_t n = read(clientFd, buffer, sizeof(buffer) - 1);
    if (n <= 0) {
        const char *msg = "REFUSED NO_DATA\n";
        ::write(clientFd, msg, strlen(msg));
        return false;
    }

    std::string resp(buffer);

    if (resp.rfind("toto", 0) != 0) {
        const char *msg = "REFUSED BAD_HANDSHAKE\n";
        ::write(clientFd, msg, strlen(msg));
        return false;
    }

    std::string ok = "OK " + std::to_string(playerId) + "\n";
    ::write(clientFd, ok.c_str(), ok.size());

    return true;
}

void TCPServer::acceptNewClient()
{
    sockaddr_in addr{};
    int clientFd = _serverSocket.acceptClient(addr);

    if (clientFd == -1)
        return;

    int slot = -1;
    for (size_t i = 0; i < _clients.size(); i++) {
        if (_clients[i].fd == -1) {
            slot = i;
            break;
        }
    }

    if (slot == -1) {
        const char *msg = "FULL\n";
        ::write(clientFd, msg, strlen(msg));
        ::close(clientFd);
        std::cout << "[SERVER] refused new client (FULL)\n";
        return;
    }

    _clients[slot].fd = clientFd;
    _clients[slot].addr = addr;
    _clients[slot].id = _nextId++;

    if (!performHandshake(clientFd, _clients[slot].id)) {
        std::cout << "[SERVER] handshake failed\n";
        ::close(clientFd);
        _clients[slot].fd = -1;
        _clients[slot].id = 0;
        return;
    }

    _clients[slot].handshakeDone = true;
    _clients[slot].lastPongTime = getCurrentTime();

    std::cout << "[SERVER] client " << _clients[slot].id << " connected successfully\n";
}

void TCPServer::processClientData(Client &client)
{
    char buffer[BUFFER_SIZE]{};
    ssize_t n = read(client.fd, buffer, sizeof(buffer));

    if (n <= 0) {
        std::cout << "[SERVER] client " << client.id << " disconnected\n";
        ::close(client.fd);
        client.fd = -1;
        client.id = 0;
        return;
    }

    std::string msg(buffer, n);

    if (msg.rfind("PONG", 0) == 0) {
        client.lastPongTime = getCurrentTime();
        std::cout << "[SERVER] Received PONG from client " << client.id << std::endl;
        return;
    }

    std::cout << "[SERVER] (" << client.id << ") said: " << msg;
}


long TCPServer::getCurrentTime()
{
    return static_cast<long>(time(nullptr));
}

void TCPServer::sendPingToAll()
{
    char *ping = "PING\n";

    for (auto &c : _clients) {
        if (c.fd != -1 && c.handshakeDone) {
            std::cout << "[SERVER] Sending PING to client " << c.id << std::endl;
            ::write(c.fd, ping, strlen(ping));
        }
    }
}

void TCPServer::checkHeartbeat()
{
    long now = getCurrentTime();

    for (auto &c : _clients) {
        if (c.fd == -1 || !c.handshakeDone)
            continue;

        if (now - c.lastPongTime > 10) {
            std::cout << "[SERVER] Client " << c.id << " timed out (no PONG for " << (now - c.lastPongTime) << "s)" << std::endl;

            ::close(c.fd);
            c.fd = -1;
            c.id = 0;
        }
    }
}


void TCPServer::run()
{
    long lastPing = getCurrentTime();

    while (true) {
        long now = getCurrentTime();
        if (now - lastPing >= 5) {
            sendPingToAll();
            checkHeartbeat();
            lastPing = now;
        }

        fd_set readfds;
        FD_ZERO(&readfds);

        int maxFd = _serverSocket.getSocketFd();
        FD_SET(_serverSocket.getSocketFd(), &readfds);

        for (auto &c : _clients) {
            if (c.fd != -1) {
                FD_SET(c.fd, &readfds);
                maxFd = std::max(maxFd, c.fd);
            }
        }

        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        int activity = select(maxFd + 1, &readfds, nullptr, nullptr, &tv);
        if (activity < 0) {
            continue;
        }

        if (activity == 0) {
            continue;
        }

        if (FD_ISSET(_serverSocket.getSocketFd(), &readfds)) {
            acceptNewClient();
        }

        for (auto &c : _clients) {
            if (c.fd != -1 && FD_ISSET(c.fd, &readfds)) {
                processClientData(c);
            }
        }
    }
}
