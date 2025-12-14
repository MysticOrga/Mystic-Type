/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** Network client (TCP + UDP)
*/

#include "NetworkClient.hpp"
#include "GameState.hpp"
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <algorithm>
#include <cstdint>
#include "../TransportLayer/Protocol.hpp"

namespace {
    constexpr size_t BUFFER_SIZE = 1024;
}

NetworkClient::NetworkClient(const std::string &ip, uint16_t port)
{
    _serverAddr.sin_family = AF_INET;
    _serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &_serverAddr.sin_addr);
}

NetworkClient::~NetworkClient()
{
    disconnect();
}

bool NetworkClient::connectToServer()
{
    _tcpFd = socket(AF_INET, SOCK_STREAM, 0);
    if (_tcpFd < 0)
        return false;
    if (::connect(_tcpFd, reinterpret_cast<sockaddr*>(&_serverAddr), sizeof(_serverAddr)) < 0)
        return false;
    _tcpConnected = true;

    _udpFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (_udpFd < 0)
        return false;
    _udpConnected = true;

    return true;
}

bool NetworkClient::performHandshake()
{
    Packet serverHello;
    if (receiveTcpFramed(serverHello) != RecvResult::Ok)
        return false;

    Packet clientHello(PacketType::CLIENT_HELLO, {'t','o','t','o'});
    if (!sendPacketTcp(clientHello))
        return false;

    Packet ok;
    if (receiveTcpFramed(ok) != RecvResult::Ok)
        return false;
    if (ok.type != PacketType::OK || ok.payload.size() < 2)
        return false;
    _playerId = (ok.payload[0] << 8) | ok.payload[1];
    return true;
}

bool NetworkClient::sendPong()
{
    Packet pong(PacketType::PONG, {});
    return sendPacketTcp(pong);
}

bool NetworkClient::sendHelloUdp(uint8_t x, uint8_t y)
{
    Packet helloUdp(PacketType::HELLO_UDP, {
        static_cast<uint8_t>((_playerId >> 8) & 0xFF),
        static_cast<uint8_t>(_playerId & 0xFF),
        x, y
    });
    return sendPacketUdp(helloUdp);
}

bool NetworkClient::sendInput(uint8_t posX, uint8_t posY, int8_t velX, int8_t velY, MoveCmd cmd)
{
    Packet input(PacketType::INPUT, {
        static_cast<uint8_t>((_playerId >> 8) & 0xFF),
        static_cast<uint8_t>(_playerId & 0xFF),
        posX,
        posY,
        static_cast<uint8_t>(velX),
        static_cast<uint8_t>(velY),
        static_cast<uint8_t>(cmd)
    });
    return sendPacketUdp(input);
}

bool NetworkClient::sendShoot(uint8_t posX, uint8_t posY, int8_t velX, int8_t velY)
{
    Packet shoot(PacketType::SHOOT, {
        static_cast<uint8_t>((_playerId >> 8) & 0xFF),
        static_cast<uint8_t>(_playerId & 0xFF),
        posX,
        posY,
        static_cast<uint8_t>(velX),
        static_cast<uint8_t>(velY)
    });
    return sendPacketUdp(shoot);
}

bool NetworkClient::sendPacketTcp(const Packet &p)
{
    std::vector<uint8_t> framed;
    try {
        framed = Protocol::frameTcp(p);
    } catch (const std::exception &) {
        return false;
    }
    return writeAll(_tcpFd, framed.data(), framed.size());
}

bool NetworkClient::sendPacketUdp(const Packet &p)
{
    auto data = p.serialize();
    ssize_t sent = sendto(_udpFd, data.data(), data.size(), 0, reinterpret_cast<sockaddr*>(&_serverAddr), sizeof(_serverAddr));
    return sent == static_cast<ssize_t>(data.size());
}

bool NetworkClient::readTcpPacket(Packet &p)
{
    auto res = receiveTcpFramed(p);
    if (res == RecvResult::Disconnected) {
        _events.push_back("TIMEOUT");
        disconnect();
    }
    return res == RecvResult::Ok;
}

bool NetworkClient::readUdpPacket(Packet &p)
{
    uint8_t buffer[BUFFER_SIZE]{};
    sockaddr_in from{};
    socklen_t len = sizeof(from);
    ssize_t n = recvfrom(_udpFd, buffer, sizeof(buffer), 0, reinterpret_cast<sockaddr*>(&from), &len);
    if (n <= 0)
        return false;
    p = Packet::deserialize(buffer, static_cast<size_t>(n));
    return true;
}

bool NetworkClient::pollPackets()
{
    fd_set rfds;
    FD_ZERO(&rfds);
    int maxFd = -1;
    if (_tcpFd != -1) {
        FD_SET(_tcpFd, &rfds);
        maxFd = std::max(maxFd, _tcpFd);
    }
    if (_udpFd != -1) {
        FD_SET(_udpFd, &rfds);
        maxFd = std::max(maxFd, _udpFd);
    }
    if (maxFd == -1)
        return false;
    struct timeval tv{0, 50000}; // 50ms

    int activity = select(maxFd + 1, &rfds, nullptr, nullptr, &tv);
    if (activity <= 0)
        return false;

    bool handled = false;
    if (_tcpFd != -1 && FD_ISSET(_tcpFd, &rfds)) {
        Packet p;
        if (readTcpPacket(p)) {
            handleTcpPacket(p);
            handled = true;
        }
    }
    if (_udpFd != -1 && FD_ISSET(_udpFd, &rfds)) {
        Packet p;
        if (readUdpPacket(p)) {
            handleUdpPacket(p);
            handled = true;
        }
    }
    return handled;
}

void NetworkClient::handleTcpPacket(const Packet &p)
{
    switch (p.type) {
        case PacketType::PING:
            sendPong();
            _events.push_back("PING");
            break;
        case PacketType::REFUSED:
            _events.push_back("REFUSED:" + std::string(p.payload.begin(), p.payload.end()));
            disconnect();
            break;
        case PacketType::PLAYER_LIST:
            _lastPlayerList.clear();
            if (!p.payload.empty()) {
                uint8_t count = p.payload[0];
                size_t expected = 1 + count * 4;
                if (p.payload.size() >= expected) {
                    for (size_t i = 0; i < count; ++i) {
                        size_t off = 1 + i * 4;
                        int id = (p.payload[off] << 8) | p.payload[off + 1];
                        uint8_t x = p.payload[off + 2];
                        uint8_t y = p.payload[off + 3];
                        _lastPlayerList.push_back({id, x, y});
                    }
                    _events.push_back("PLAYER_LIST");
                }
            }
            break;
        case PacketType::NEW_PLAYER:
            if (p.payload.size() >= 4) {
                int id = (p.payload[0] << 8) | p.payload[1];
                uint8_t x = p.payload[2];
                uint8_t y = p.payload[3];
                _lastPlayerList.push_back({id, x, y});
                _events.push_back("NEW_PLAYER");
            }
            break;
        case PacketType::MESSAGE:
            _events.push_back("MESSAGE:" + std::string(p.payload.begin(), p.payload.end()));
            break;
        default:
            break;
    }
}

void NetworkClient::handleUdpPacket(const Packet &p)
{
    if (p.type == PacketType::SNAPSHOT && !p.payload.empty()) {
        _lastSnapshot.clear();
        _lastSnapshotBullets.clear();
        _lastSnapshotMonsters.clear();
        uint8_t count = p.payload[0];
        size_t off = 1;
        size_t expectedPlayers = off + count * 4;
        if (p.payload.size() < expectedPlayers)
            return;

        for (size_t i = 0; i < count; ++i) {
            size_t idx = off + i * 4;
            int id = (p.payload[idx] << 8) | p.payload[idx + 1];
            uint8_t x = p.payload[idx + 2];
            uint8_t y = p.payload[idx + 3];
            _lastSnapshot.push_back({id, x, y});
        }

        off = expectedPlayers;
        if (off >= p.payload.size()) {
            _events.push_back("SNAPSHOT");
            return;
        }

        uint8_t bulletCount = p.payload[off++];
        size_t expectedBullets = off + bulletCount * 6;
        if (p.payload.size() < expectedBullets)
            return;
        for (size_t i = 0; i < bulletCount; ++i) {
            size_t idx = off + i * 6;
            int id = (p.payload[idx] << 8) | p.payload[idx + 1];
            uint8_t x = p.payload[idx + 2];
            uint8_t y = p.payload[idx + 3];
            int8_t vx = static_cast<int8_t>(p.payload[idx + 4]);
            int8_t vy = static_cast<int8_t>(p.payload[idx + 5]);
            _lastSnapshotBullets.push_back({id, x, y, vx, vy});
        }

        off = expectedBullets;
        if (off < p.payload.size()) {
            uint8_t monsterCount = p.payload[off++];
            size_t expectedMonsters = off + monsterCount * 6;
            if (p.payload.size() < expectedMonsters)
                return;
            for (size_t i = 0; i < monsterCount; ++i) {
                size_t idx = off + i * 6;
                int id = (p.payload[idx] << 8) | p.payload[idx + 1];
                uint8_t x = p.payload[idx + 2];
                uint8_t y = p.payload[idx + 3];
                uint8_t hp = p.payload[idx + 4];
                uint8_t type = p.payload[idx + 5];
                _lastSnapshotMonsters.push_back({id, x, y, hp, type});
            }
        }

        _events.push_back("SNAPSHOT");
    }
}

bool NetworkClient::writeAll(int fd, const uint8_t *data, std::size_t size)
{
    std::size_t total = 0;
    while (total < size) {
        ssize_t n = ::write(fd, data + total, size - total);
        if (n <= 0)
            return false;
        total += static_cast<std::size_t>(n);
    }
    return true;
}

NetworkClient::RecvResult NetworkClient::receiveTcpFramed(Packet &p)
{
    uint8_t tmp[BUFFER_SIZE]{};
    ssize_t n = ::read(_tcpFd, tmp, sizeof(tmp));
    if (n <= 0)
        return RecvResult::Disconnected;

    auto status = Protocol::consumeChunk(tmp, static_cast<std::size_t>(n), _tcpRecvBuffer, p);
    if (status == Protocol::StreamStatus::Ok)
        return RecvResult::Ok;
    if (status == Protocol::StreamStatus::Incomplete)
        return RecvResult::Incomplete;
    return RecvResult::Disconnected;
}

void NetworkClient::disconnect()
{
    if (_tcpFd != -1) {
        ::close(_tcpFd);
        _tcpFd = -1;
    }
    if (_udpFd != -1) {
        ::close(_udpFd);
        _udpFd = -1;
    }
    _tcpConnected = false;
    _udpConnected = false;
}
