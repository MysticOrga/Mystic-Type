/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** TCPServer
*/

#include "TCPServer.hpp"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/select.h>
#include "../Protocol.hpp"

TCPServer::TCPServer(uint16_t port, SessionManager &sessions)
    : _sessions(sessions)
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

bool TCPServer::sendPacket(socket_t fd, const Packet &packet)
{
    if (fd == INVALID_SOCKET_FD) {
        return false;
    }
    std::vector<uint8_t> framed;
    try {
        framed = Protocol::frameTcp(packet);
    } catch (const std::exception &) {
        return false;
    }
    return writeAll(fd, framed.data(), framed.size());
}

TCPServer::RecvResult TCPServer::receivePacket(socket_t fd, Packet &packet, std::vector<uint8_t> &recvBuffer)
{
    if (fd == INVALID_SOCKET_FD) {
        return RecvResult::Disconnected;
    }

    uint8_t tmp[BUFFER_SIZE]{};
    ssize_t n = readFd(fd, tmp, sizeof(tmp));
    if (n <= 0) {
        return RecvResult::Disconnected;
    }

    recvBuffer.insert(recvBuffer.end(), tmp, tmp + n);

    auto status = Protocol::consumeChunk(tmp, static_cast<std::size_t>(n), recvBuffer, packet);
    if (status == Protocol::StreamStatus::Ok)
        return RecvResult::Ok;
    if (status == Protocol::StreamStatus::Incomplete)
        return RecvResult::Incomplete;
    return RecvResult::Disconnected;
}

bool TCPServer::waitForReadable(socket_t fd, int timeoutSec, int timeoutUsec)
{
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);

    struct timeval timeout{timeoutSec, timeoutUsec};

    int ret = selectFdSet(fd + 1, &rfds, nullptr, nullptr, &timeout);
    return ret > 0 && FD_ISSET(fd, &rfds);
}

void TCPServer::closeFd(socket_t &fd)
{
    if (fd != INVALID_SOCKET_FD) {
        closeFdRaw(fd);
        fd = INVALID_SOCKET_FD;
    }
}

void TCPServer::resetClient(Client &client)
{
    _sessions.removeById(client.id);
    closeFd(client.fd);
    client.id = 0;
    client.addr = {};
    client.handshakeDone = false;
    client.lastPongTime = 0;
    client.posX = 0;
    client.posY = 0;
    client.recvBuffer.clear();
}

int TCPServer::pollSockets(fd_set &readfds, int maxFd, struct timeval &timeout)
{
    return selectFdSet(maxFd + 1, &readfds, nullptr, nullptr, &timeout);
}

bool TCPServer::performHandshake(Client &client)
{
    Packet serverHello = makeStringPacket(PacketType::SERVER_HELLO, "R-Type Server");
    if (!sendPacket(client.fd, serverHello)) {
        return false;
    }

    if (!waitForReadable(client.fd, 3)) {
        sendPacket(client.fd, makeStringPacket(PacketType::REFUSED, "TIMEOUT"));
        return false;
    }

    Packet resp;
    try {
        auto res = receivePacket(client.fd, resp, client.recvBuffer);
        if (res == RecvResult::Disconnected) {
            sendPacket(client.fd, makeStringPacket(PacketType::REFUSED, "NO_DATA"));
            return false;
        } else if (res == RecvResult::Incomplete) {
            sendPacket(client.fd, makeStringPacket(PacketType::REFUSED, "BAD_PACKET"));
            return false;
        }
    } catch (const std::exception &e) {
        sendPacket(client.fd, makeStringPacket(PacketType::REFUSED, "BAD_PACKET"));
        return false;
    }

    std::string payloadStr(resp.payload.begin(), resp.payload.end());
    if (resp.type != PacketType::CLIENT_HELLO || payloadStr.rfind("toto", 0) != 0) {
        sendPacket(client.fd, makeStringPacket(PacketType::REFUSED, "BAD_HANDSHAKE"));
        return false;
    }

    sendPacket(client.fd, makeIdPacket(PacketType::OK, client.id));

    return true;
}

void TCPServer::acceptNewClient()
{
    sockaddr_in addr{};
    socket_t clientFd = _serverSocket.acceptClient(addr);

    if (clientFd == INVALID_SOCKET_FD)
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
    _clients[slot].posX = static_cast<uint8_t>(slot);
    _clients[slot].posY = 0;
    _sessions.addSession(_clients[slot].id, clientFd, addr, getCurrentTime());

    if (!performHandshake(_clients[slot])) {
        std::cout << "[SERVER] handshake failed\n";
        resetClient(_clients[slot]);
        return;
    }

    _clients[slot].handshakeDone = true;
    _clients[slot].lastPongTime = getCurrentTime();

    std::cout << "[SERVER] client " << _clients[slot].id << " connected successfully\n";
    sendPlayerListToClient(_clients[slot]);
    broadcastNewPlayer(_clients[slot]);
}

void TCPServer::processClientData(Client &client)
{
    Packet packet;
    auto res = receivePacket(client.fd, packet, client.recvBuffer);
    if (res == RecvResult::Disconnected) {
        std::cout << "[SERVER] client " << client.id << " disconnected\n";
        resetClient(client);
        return;
    }
    if (res == RecvResult::Incomplete) {
        return;
    }

    if (packet.type == PacketType::PONG) {
        client.lastPongTime = getCurrentTime();
        _sessions.updatePong(client.id, client.lastPongTime);
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

ssize_t TCPServer::writeFd(socket_t fd, const uint8_t *data, std::size_t size)
{
    return ::write(fd, data, size);
}

ssize_t TCPServer::readFd(socket_t fd, uint8_t *data, std::size_t size)
{
    return ::read(fd, data, size);
}

int TCPServer::selectFdSet(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
    return ::select(nfds, readfds, writefds, exceptfds, timeout);
}

int TCPServer::closeFdRaw(socket_t fd)
{
    return CLOSE(fd);
}

bool TCPServer::writeAll(socket_t fd, const uint8_t *data, std::size_t size)
{
    std::size_t total = 0;
    while (total < size) {
        ssize_t n = writeFd(fd, data + total, size - total);
        if (n <= 0) {
            return false;
        }
        total += static_cast<std::size_t>(n);
    }
    return true;
}
Packet TCPServer::buildPlayerListPacket() const
{
    std::vector<uint8_t> payload;
    payload.reserve(1 + _clients.size() * 6);

    payload.push_back(0); // placeholder for count
    uint8_t count = 0;

    for (const auto &c : _clients) {
        if (c.fd == -1 || !c.handshakeDone)
            continue;
        ++count;
        payload.push_back(static_cast<uint8_t>((c.id >> 8) & 0xFF));
        payload.push_back(static_cast<uint8_t>(c.id & 0xFF));
        payload.push_back(c.posX);
        payload.push_back(c.posY);
    }

    payload[0] = count;
    return Packet(PacketType::PLAYER_LIST, payload);
}

void TCPServer::sendPlayerListToClient(const Client &client)
{
    Packet list = buildPlayerListPacket();
    sendPacket(client.fd, list);
}

void TCPServer::broadcastNewPlayer(const Client &newClient)
{
    std::vector<uint8_t> payload{
        static_cast<uint8_t>((newClient.id >> 8) & 0xFF),
        static_cast<uint8_t>(newClient.id & 0xFF),
        newClient.posX,
        newClient.posY
    };
    Packet pkt(PacketType::NEW_PLAYER, payload);

    for (auto &c : _clients) {
        if (c.fd == -1 || !c.handshakeDone || c.id == newClient.id)
            continue;
        sendPacket(c.fd, pkt);
    }
}
