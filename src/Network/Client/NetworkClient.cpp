/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** Network client (TCP + UDP)
*/

#include "NetworkClient.hpp"
#include "GameState.hpp"
#include <iostream>
#include <algorithm>
#include <cstdint>
#include <netinet/tcp.h>
#include "../TransportLayer/Protocol.hpp"

namespace {
    constexpr size_t BUFFER_SIZE = 1024;
}

NetworkClient::NetworkClient(const std::string &ip, uint16_t port)
{
    _tcpAddr.sin_family = AF_INET;
    _tcpAddr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &_tcpAddr.sin_addr);
    _udpAddr = _tcpAddr;
}

NetworkClient::~NetworkClient()
{
    disconnect();
}

bool NetworkClient::connectToServer()
{
    _tcpFd = socket(AF_INET, SOCK_STREAM, 0);
    std::cout << "TCP fd: " << _tcpFd << std::endl;
    if (_tcpFd < 0)
        return false;
    if (::connect(_tcpFd, reinterpret_cast<sockaddr*>(&_tcpAddr), sizeof(_tcpAddr)) < 0)
        return false;
    char flag = 1;
    setsockopt(_tcpFd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));
    _tcpConnected = true;

    _udpFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (_udpFd < 0)
        return false;
    std::cout << "UDP connected " << std::endl;
    _udpConnected = true;

    return true;
}

bool NetworkClient::performHandshake()
{
    Packet serverHello;
    if (receiveTcpFramed(serverHello) != RecvResult::Ok)
        return false;

    auto sanitizePseudo = [](const std::string &raw) {
        std::string out;
        out.reserve(raw.size());
        for (char c : raw) {
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
                || (c >= '0' && c <= '9') || c == '_' || c == '-') {
                out.push_back(c);
            }
            if (out.size() >= 12)
                break;
        }
        return out;
    };
    std::string cleanPseudo = sanitizePseudo(_pseudo);
    std::string helloPayload = "toto";
    if (!cleanPseudo.empty()) {
        helloPayload += "|" + cleanPseudo;
    } else {
        helloPayload += "|";
    }
    Packet clientHello(PacketType::CLIENT_HELLO, std::vector<uint8_t>(helloPayload.begin(), helloPayload.end()));
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

bool NetworkClient::sendChat(const std::string &text)
{
    std::string clean;
    clean.reserve(text.size());
    for (char c : text) {
        if (c >= 32 && c <= 126) {
            clean.push_back(c);
        }
        if (clean.size() >= 120)
            break;
    }
    if (clean.empty())
        return false;
    std::cout << "[CLIENT] sendChat \"" << clean << "\"\n";
    Packet msg(PacketType::MESSAGE, std::vector<uint8_t>(clean.begin(), clean.end()));
    bool ok = sendPacketTcp(msg);
    if (!ok) {
        std::cout << "[CLIENT] sendChat failed\n";
    }
    return ok;
}

bool NetworkClient::sendCreateLobby()
{
    std::cout << "[CLIENT] SEND CREATE_LOBBY\n";
    Packet pkt(PacketType::CREATE_LOBBY, {});
    return sendPacketTcp(pkt);
}

bool NetworkClient::sendJoinLobby(const std::string &code)
{
    std::cout << "[CLIENT] SEND JOIN_LOBBY " << code << "\n";
    Packet pkt(PacketType::JOIN_LOBBY, {code.begin(), code.end()});
    return sendPacketTcp(pkt);
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
    ssize_t sent = sendto(_udpFd, reinterpret_cast<const char *>(data.data()), data.size(), 0, reinterpret_cast<sockaddr*>(&_udpAddr), sizeof(_udpAddr));
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
    ssize_t n = recvfrom(_udpFd, reinterpret_cast<char *>(buffer), sizeof(buffer), 0, reinterpret_cast<sockaddr*>(&from), &len);
    if (n <= 0)
        return false;
    p = Packet::deserialize(buffer, static_cast<size_t>(n));
    return true;
}

bool NetworkClient::pollPackets()
{
    fd_set rfds, wfds, efds;
    FD_ZERO(&rfds);
    socket_t maxFd = 0;
    std::cout << "Maxfd: " << maxFd << std::endl;
    if (_tcpFd != INVALID_SOCKET_FD) {
        FD_SET(_tcpFd, &rfds);
        maxFd = std::max(maxFd, _tcpFd);
    }
    if (_udpFd != INVALID_SOCKET_FD) {
        FD_SET(_udpFd, &rfds);
        maxFd = std::max(maxFd, _udpFd);
    }
    std::cout << "UDPfd: " << _udpFd << std::endl;
    std::cout << "tcpfd: " << _tcpFd << std::endl;
    std::cout << "Maxfd: " << maxFd << std::endl;
    if (maxFd == INVALID_SOCKET_FD)
        return false;
    struct timeval tv{0, 0}; // non-blocking

    int activity = select(maxFd + 1, &rfds, nullptr, nullptr, &tv);
    if (activity < 0)
        return false;

    bool handled = false;
    if (_tcpFd != INVALID_SOCKET_FD && FD_ISSET(_tcpFd, &rfds)) {
        Packet p;
        if (readTcpPacket(p)) {
            handleTcpPacket(p);
            Packet extra;
            while (Protocol::extractFromBuffer(_tcpRecvBuffer, extra) == Protocol::StreamStatus::Ok) {
                handleTcpPacket(extra);
            }
            handled = true;
        }
    }
    if (_udpFd != INVALID_SOCKET_FD && FD_ISSET(_udpFd, &rfds)) {
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
            if (sendPong()) {
                _events.push_back("PING");
            } else {
                _events.push_back("PING_SEND_FAIL");
            }
            break;
        case PacketType::REFUSED:
            _events.push_back("REFUSED:" + std::string(p.payload.begin(), p.payload.end()));
            disconnect();
            break;
        case PacketType::PLAYER_LIST:
            _lastPlayerList.clear();
            if (!p.payload.empty()) {
                uint8_t count = p.payload[0];
                size_t expected = 1 + count * 5;
                if (p.payload.size() >= expected) {
                    for (size_t i = 0; i < count; ++i) {
                        size_t off = 1 + i * 5;
                        int id = (p.payload[off] << 8) | p.payload[off + 1];
                        uint8_t x = p.payload[off + 2];
                        uint8_t y = p.payload[off + 3];
                        uint8_t hp = p.payload[off + 4];
                        _lastPlayerList.push_back({id, x, y, hp, 0});
                    }
                    _events.push_back("PLAYER_LIST");
                }
            }
            break;
        case PacketType::NEW_PLAYER:
            if (p.payload.size() >= 5) {
                int id = (p.payload[0] << 8) | p.payload[1];
                uint8_t x = p.payload[2];
                uint8_t y = p.payload[3];
                uint8_t hp = p.payload[4];
                _lastPlayerList.push_back({id, x, y, hp, 0});
                _events.push_back("NEW_PLAYER");
            }
            break;
        case PacketType::LOBBY_OK:
        {
            // Payload format: CODE|PORT (simple ASCII) for now.
            std::string payloadStr(p.payload.begin(), p.payload.end());
            auto sep = payloadStr.find('|');
            if (sep == std::string::npos) {
                _lobbyCode = payloadStr;
            } else {
                _lobbyCode = payloadStr.substr(0, sep);
                std::string portStr = payloadStr.substr(sep + 1);
                int port = std::atoi(portStr.c_str());
                if (port > 0 && port < 65536) {
                    _udpAddr.sin_port = htons(static_cast<uint16_t>(port));
                }
            }
            _events.push_back("LOBBY_OK:" + _lobbyCode);
            break;
        }
        case PacketType::LOBBY_ERROR:
            _events.push_back("LOBBY_ERROR:" + std::string(p.payload.begin(), p.payload.end()));
            break;
        case PacketType::MESSAGE:
            std::cout << "[CLIENT] recv MESSAGE \"" << std::string(p.payload.begin(), p.payload.end()) << "\"\n";
            _events.push_back("MESSAGE:" + std::string(p.payload.begin(), p.payload.end()));
            break;
        default:
            break;
    }
}

void NetworkClient::handleUdpPacket(const Packet &p)
{
    if (p.type == PacketType::SNAPSHOT && !p.payload.empty()) {
        auto parseSnapshot = [&](size_t perPlayer, bool hasScore,
                                 std::vector<PlayerState> &players,
                                 std::vector<BulletState> &bullets,
                                 std::vector<MonsterState> &monsters) -> bool {
            uint8_t count = p.payload[0];
            size_t off = 1;
            size_t expectedPlayers = off + count * perPlayer;
            if (p.payload.size() < expectedPlayers)
                return false;

            for (size_t i = 0; i < count; ++i) {
                size_t idx = off + i * perPlayer;
                int id = (p.payload[idx] << 8) | p.payload[idx + 1];
                uint8_t x = p.payload[idx + 2];
                uint8_t y = p.payload[idx + 3];
                uint8_t hp = p.payload[idx + 4];
                uint16_t score = 0;
                if (hasScore) {
                    score = (p.payload[idx + 5] << 8) | p.payload[idx + 6];
                }
                players.push_back({id, x, y, hp, score});
            }

            off = expectedPlayers;
            if (off >= p.payload.size())
                return true;

            uint8_t bulletCount = p.payload[off++];
            size_t expectedBullets = off + bulletCount * 6;
            if (p.payload.size() < expectedBullets)
                return false;
            for (size_t i = 0; i < bulletCount; ++i) {
                size_t idx = off + i * 6;
                int id = (p.payload[idx] << 8) | p.payload[idx + 1];
                uint8_t x = p.payload[idx + 2];
                uint8_t y = p.payload[idx + 3];
                int8_t vx = static_cast<int8_t>(p.payload[idx + 4]);
                int8_t vy = static_cast<int8_t>(p.payload[idx + 5]);
                bullets.push_back({id, x, y, vx, vy});
            }

            off = expectedBullets;
            if (off < p.payload.size()) {
                uint8_t monsterCount = p.payload[off++];
                size_t expectedMonsters = off + monsterCount * 6;
                if (p.payload.size() < expectedMonsters)
                    return false;
                for (size_t i = 0; i < monsterCount; ++i) {
                    size_t idx = off + i * 6;
                    int id = (p.payload[idx] << 8) | p.payload[idx + 1];
                    uint8_t x = p.payload[idx + 2];
                    uint8_t y = p.payload[idx + 3];
                    uint8_t hp = p.payload[idx + 4];
                    uint8_t type = p.payload[idx + 5];
                    monsters.push_back({id, x, y, hp, type});
                }
            }

            return true;
        };

        std::vector<PlayerState> players;
        std::vector<BulletState> bullets;
        std::vector<MonsterState> monsters;
        if (!parseSnapshot(7, true, players, bullets, monsters)) {
            players.clear();
            bullets.clear();
            monsters.clear();
            if (!parseSnapshot(5, false, players, bullets, monsters))
                return;
        }

        _lastSnapshot = std::move(players);
        _lastSnapshotBullets = std::move(bullets);
        _lastSnapshotMonsters = std::move(monsters);
        _events.push_back("SNAPSHOT");
    }
}

bool NetworkClient::writeAll(socket_t fd, const uint8_t *data, std::size_t size)
{
    std::size_t total = 0;
    while (total < size) {
        ssize_t n = send(fd, reinterpret_cast<const char *>(data + total), size - total, 0);
        if (n <= 0) {
            std::cerr << "FALSE" << std::endl;
            return false;
        }
        total += static_cast<std::size_t>(n);
    }
    return true;
}

NetworkClient::RecvResult NetworkClient::receiveTcpFramed(Packet &p)
{
    uint8_t tmp[BUFFER_SIZE]{};
    std::cout << "before tcp receive" << std::endl;
    ssize_t n = recv(_tcpFd, reinterpret_cast<char *>(tmp), sizeof(tmp), 0);
    std::cout << "after tcp receive" << std::endl;
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
        CLOSE(_tcpFd);
        _tcpFd = -1;
    }
    if (_udpFd != -1) {
        CLOSE(_udpFd);
        _udpFd = -1;
    }
    _tcpConnected = false;
    _udpConnected = false;
    _lobbyCode.clear();
}

void NetworkClient::resetForReconnect()
{
    _playerId = -1;
    _lobbyCode.clear();
    _events.clear();
    _lastSnapshot.clear();
    _lastSnapshotBullets.clear();
    _lastSnapshotMonsters.clear();
    _lastPlayerList.clear();
    _tcpRecvBuffer.clear();
}
