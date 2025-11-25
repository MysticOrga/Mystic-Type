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
    if (_tcpFd != -1)
        close(_tcpFd);
    if (_udpFd != -1)
        close(_udpFd);
}

bool NetworkClient::connectToServer()
{
    _tcpFd = socket(AF_INET, SOCK_STREAM, 0);
    if (_tcpFd < 0)
        return false;
    if (::connect(_tcpFd, reinterpret_cast<sockaddr*>(&_serverAddr), sizeof(_serverAddr)) < 0)
        return false;

    _udpFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (_udpFd < 0)
        return false;

    return true;
}

bool NetworkClient::performHandshake()
{
    uint8_t buffer[BUFFER_SIZE]{};
    ssize_t n = ::read(_tcpFd, buffer, sizeof(buffer));
    if (n <= 0)
        return false;
    Packet serverHello = Packet::deserialize(buffer, n);

    Packet clientHello(PacketType::CLIENT_HELLO, {'t','o','t','o'});
    if (!sendPacketTcp(clientHello))
        return false;

    n = ::read(_tcpFd, buffer, sizeof(buffer));
    if (n <= 0)
        return false;
    Packet ok = Packet::deserialize(buffer, n);
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

bool NetworkClient::sendInput(uint8_t x, uint8_t y)
{
    Packet input(PacketType::INPUT, {
        static_cast<uint8_t>((_playerId >> 8) & 0xFF),
        static_cast<uint8_t>(_playerId & 0xFF),
        x, y
    });
    return sendPacketUdp(input);
}

bool NetworkClient::sendPacketTcp(const Packet &p)
{
    auto data = p.serialize();
    ssize_t sent = ::write(_tcpFd, data.data(), data.size());
    return sent == static_cast<ssize_t>(data.size());
}

bool NetworkClient::sendPacketUdp(const Packet &p)
{
    auto data = p.serialize();
    ssize_t sent = sendto(_udpFd, data.data(), data.size(), 0,
                          reinterpret_cast<sockaddr*>(&_serverAddr), sizeof(_serverAddr));
    return sent == static_cast<ssize_t>(data.size());
}

bool NetworkClient::readTcpPacket(Packet &p)
{
    uint8_t buffer[BUFFER_SIZE]{};
    ssize_t n = ::read(_tcpFd, buffer, sizeof(buffer));
    if (n <= 0)
        return false;
    p = Packet::deserialize(buffer, static_cast<size_t>(n));
    return true;
}

bool NetworkClient::readUdpPacket(Packet &p)
{
    uint8_t buffer[BUFFER_SIZE]{};
    sockaddr_in from{};
    socklen_t len = sizeof(from);
    ssize_t n = recvfrom(_udpFd, buffer, sizeof(buffer), 0,
                         reinterpret_cast<sockaddr*>(&from), &len);
    if (n <= 0)
        return false;
    p = Packet::deserialize(buffer, static_cast<size_t>(n));
    return true;
}

bool NetworkClient::pollPackets()
{
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(_tcpFd, &rfds);
    FD_SET(_udpFd, &rfds);
    int maxFd = std::max(_tcpFd, _udpFd);
    struct timeval tv{0, 50000}; // 50ms

    int activity = select(maxFd + 1, &rfds, nullptr, nullptr, &tv);
    if (activity <= 0)
        return false;

    bool handled = false;
    if (FD_ISSET(_tcpFd, &rfds)) {
        Packet p;
        if (readTcpPacket(p)) {
            handleTcpPacket(p);
            handled = true;
        }
    }
    if (FD_ISSET(_udpFd, &rfds)) {
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
        default:
            break;
    }
}

void NetworkClient::handleUdpPacket(const Packet &p)
{
    if (p.type == PacketType::SNAPSHOT && !p.payload.empty()) {
        _lastSnapshot.clear();
        uint8_t count = p.payload[0];
        size_t expected = 1 + count * 4;
        if (p.payload.size() >= expected) {
            for (size_t i = 0; i < count; ++i) {
                size_t off = 1 + i * 4;
                int id = (p.payload[off] << 8) | p.payload[off + 1];
                uint8_t x = p.payload[off + 2];
                uint8_t y = p.payload[off + 3];
                _lastSnapshot.push_back({id, x, y});
            }
            _events.push_back("SNAPSHOT");
        }
    }
}
