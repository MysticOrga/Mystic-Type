/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** TCPServer
*/

#include "TCPServer.hpp"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <string>
#include <unistd.h>
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
        resetClient(c);
    }
    _serverSocket.closeSocket();
}

Packet TCPServer::makeStringPacket(PacketType type, const std::string &payload)
{
    return Packet{type, std::vector<uint8_t>(payload.begin(), payload.end())};
}

Packet TCPServer::makeIdPacket(PacketType type, int value)
{
    std::vector<uint8_t> payload{
        static_cast<uint8_t>((value >> 8) & 0xFF),
        static_cast<uint8_t>(value & 0xFF)
    };
    return Packet{type, payload};
}

bool TCPServer::sendPacket(int fd, const Packet &packet)
{
    if (fd == -1) {
        return false;
    }
    std::vector<uint8_t> buffer = packet.serialize();
    ssize_t sent = writeFd(fd, buffer.data(), buffer.size());
    return sent == static_cast<ssize_t>(buffer.size());
}

bool TCPServer::receivePacket(int fd, Packet &packet)
{
    if (fd == -1) {
        return false;
    }

    uint8_t buffer[BUFFER_SIZE]{};
    ssize_t n = readFd(fd, buffer, sizeof(buffer));
    if (n <= 0) {
        return false;
    }
    packet = Packet::deserialize(buffer, static_cast<size_t>(n));
    return true;
}

bool TCPServer::waitForReadable(int fd, int timeoutSec, int timeoutUsec)
{
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);

    struct timeval timeout{timeoutSec, timeoutUsec};

    int ret = selectFdSet(fd + 1, &rfds, nullptr, nullptr, &timeout);
    return ret > 0 && FD_ISSET(fd, &rfds);
}

void TCPServer::closeFd(int &fd)
{
    if (fd != -1) {
        closeFdRaw(fd);
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
    return selectFdSet(maxFd + 1, &readfds, nullptr, nullptr, &timeout);
}

bool TCPServer::performHandshake(int clientFd, int playerId)
{
    Packet serverHello = makeStringPacket(PacketType::SERVER_HELLO, "R-Type Server");
    if (!sendPacket(clientFd, serverHello)) {
        return false;
    }

    if (!waitForReadable(clientFd, 3)) {
        sendPacket(clientFd, makeStringPacket(PacketType::REFUSED, "TIMEOUT"));
        return false;
    }

    Packet resp;
    try {
        if (!receivePacket(clientFd, resp)) {
            sendPacket(clientFd, makeStringPacket(PacketType::REFUSED, "NO_DATA"));
            return false;
        }
    } catch (const std::exception &e) {
        sendPacket(clientFd, makeStringPacket(PacketType::REFUSED, "BAD_PACKET"));
        return false;
    }

    std::string payloadStr(resp.payload.begin(), resp.payload.end());
    if (resp.type != PacketType::CLIENT_HELLO || payloadStr.rfind("toto", 0) != 0) {
        sendPacket(clientFd, makeStringPacket(PacketType::REFUSED, "BAD_HANDSHAKE"));
        return false;
    }

    sendPacket(clientFd, makeIdPacket(PacketType::OK, playerId));

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
        sendPacket(clientFd, makeStringPacket(PacketType::REFUSED, "FULL"));
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
    Packet packet;
    try {
        if (!receivePacket(client.fd, packet)) {
            std::cout << "[SERVER] client " << client.id << " disconnected\n";
            resetClient(client);
            return;
        }
    } catch (const std::exception &e) {
        std::cout << "[SERVER] client " << client.id << " sent invalid packet: " << e.what() << std::endl;
        resetClient(client);
        return;
    }

    if (packet.type == PacketType::PONG) {
        client.lastPongTime = getCurrentTime();
        std::cout << "[SERVER] Received PONG from client " << client.id << std::endl;
        return;
    }

    std::string payloadStr(packet.payload.begin(), packet.payload.end());
    std::cout << "[SERVER] (" << client.id << ") packet type " << static_cast<int>(packet.type)
            << " payload: " << payloadStr << std::endl;
}


long TCPServer::getCurrentTime()
{
    using namespace std::chrono;
    return duration_cast<seconds>(steady_clock::now().time_since_epoch()).count();
}


void TCPServer::sendPingToAll()
{
    for (auto &c : _clients) {
        if (c.fd != -1 && c.handshakeDone) {
            std::cout << "[SERVER] Sending PING to client " << c.id << std::endl;
            sendPacket(c.fd, Packet(PacketType::PING, {}));
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

ssize_t TCPServer::writeFd(int fd, const uint8_t *data, std::size_t size)
{
    return ::write(fd, data, size);
}

ssize_t TCPServer::readFd(int fd, uint8_t *data, std::size_t size)
{
    return ::read(fd, data, size);
}

int TCPServer::selectFdSet(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
    return ::select(nfds, readfds, writefds, exceptfds, timeout);
}

int TCPServer::closeFdRaw(int fd)
{
    return ::close(fd);
}
