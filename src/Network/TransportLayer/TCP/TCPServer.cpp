/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** TCPServer
*/

#include "TCPServer.hpp"
#include <algorithm>
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <sys/select.h>
#include <string_view>

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
        resetClient(c);
    }
    _serverSocket.closeSocket();
}

bool TCPServer::sendMessage(int fd, std::string_view message)
{
    if (fd == -1) {
        return false;
    }
    ssize_t sent = ::write(fd, message.data(), message.size());
    return sent == static_cast<ssize_t>(message.size());
}

bool TCPServer::receiveMessage(int fd, std::string &message)
{
    if (fd == -1) {
        return false;
    }

    char buffer[BUFFER_SIZE]{};
    ssize_t n = ::read(fd, buffer, sizeof(buffer));
    if (n <= 0) {
        return false;
    }
    message.assign(buffer, static_cast<size_t>(n));
    return true;
}

bool TCPServer::waitForReadable(int fd, int timeoutSec, int timeoutUsec)
{
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);

    struct timeval timeout{timeoutSec, timeoutUsec};

    int ret = ::select(fd + 1, &rfds, nullptr, nullptr, &timeout);
    return ret > 0 && FD_ISSET(fd, &rfds);
}

void TCPServer::closeFd(int &fd)
{
    if (fd != -1) {
        ::close(fd);
        fd = -1;
    }
}

void TCPServer::resetClient(Client &client)
{
    closeFd(client.fd);
    client.id = 0;
    client.addr = {};
    client.handshakeDone = false;
    client.lastPongTime = 0;
}

int TCPServer::pollSockets(fd_set &readfds, int maxFd, struct timeval &timeout)
{
    return ::select(maxFd + 1, &readfds, nullptr, nullptr, &timeout);
}

bool TCPServer::performHandshake(int clientFd, int playerId)
{
    if (!sendMessage(clientFd, "R-Type Server\n")) {
        return false;
    }

    if (!waitForReadable(clientFd, 3)) {
        sendMessage(clientFd, "REFUSED TIMEOUT\n");
        return false;
    }

    std::string resp;
    if (!receiveMessage(clientFd, resp)) {
        sendMessage(clientFd, "REFUSED NO_DATA\n");
        return false;
    }

    if (resp.rfind("toto", 0) != 0) {
        sendMessage(clientFd, "REFUSED BAD_HANDSHAKE\n");
        return false;
    }

    std::string ok = "OK " + std::to_string(playerId) + "\n";
    sendMessage(clientFd, ok);

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
        sendMessage(clientFd, "FULL\n");
        closeFd(clientFd);
        std::cout << "[SERVER] refused new client (FULL)\n";
        return;
    }

    _clients[slot].fd = clientFd;
    _clients[slot].addr = addr;
    _clients[slot].id = _nextId++;

    if (!performHandshake(clientFd, _clients[slot].id)) {
        std::cout << "[SERVER] handshake failed\n";
        resetClient(_clients[slot]);
        return;
    }

    _clients[slot].handshakeDone = true;
    _clients[slot].lastPongTime = getCurrentTime();

    std::cout << "[SERVER] client " << _clients[slot].id << " connected successfully\n";
}

void TCPServer::processClientData(Client &client)
{
    std::string msg;
    if (!receiveMessage(client.fd, msg)) {
        std::cout << "[SERVER] client " << client.id << " disconnected\n";
        resetClient(client);
        return;
    }

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
    for (auto &c : _clients) {
        if (c.fd != -1 && c.handshakeDone) {
            std::cout << "[SERVER] Sending PING to client " << c.id << std::endl;
            sendMessage(c.fd, "PING\n");
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

            resetClient(c);
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

        int activity = pollSockets(readfds, maxFd, tv);
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
