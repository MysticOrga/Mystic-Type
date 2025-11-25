/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** UDP Game Server
*/

#include "UDPGameServer.hpp"
#include <iostream>
#include <thread>

namespace {
    constexpr std::size_t UDP_BUFFER_SIZE = 1024;
}

UDPGameServer::UDPGameServer(uint16_t port, long long snapshotIntervalMs)
    : _port(port), _snapshotIntervalMs(snapshotIntervalMs)
{
    if (!_socket.bindTo(port)) {
        throw std::runtime_error("Failed to bind UDP socket");
    }
    std::cout << "[UDP] Listening on port " << port << std::endl;
}

long long UDPGameServer::nowMs() const
{
    using namespace std::chrono;
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

bool UDPGameServer::sendPacketTo(const Packet &packet, const sockaddr_in &to)
{
    auto data = packet.serialize();
    return _socket.writeByte(data.data(), data.size(), to) == static_cast<ssize_t>(data.size());
}

Packet UDPGameServer::buildSnapshotPacket() const
{
    std::vector<uint8_t> payload;
    payload.reserve(1 + _players.size() * 4);

    payload.push_back(static_cast<uint8_t>(_players.size()));
    for (const auto &kv : _players) {
        const auto &p = kv.second;
        payload.push_back(static_cast<uint8_t>((p.id >> 8) & 0xFF));
        payload.push_back(static_cast<uint8_t>(p.id & 0xFF));
        payload.push_back(p.x);
        payload.push_back(p.y);
    }
    return Packet(PacketType::SNAPSHOT, payload);
}

void UDPGameServer::broadcastSnapshot()
{
    Packet snap = buildSnapshotPacket();
    for (const auto &kv : _players) {
        sendPacketTo(snap, kv.second.addr);
    }
}

void UDPGameServer::handleHello(const Packet &packet, const sockaddr_in &from)
{
    if (packet.payload.size() < 2) {
        std::cerr << "[UDP] HELLO_UDP payload too small\n";
        return;
    }
    int id = (packet.payload[0] << 8) | packet.payload[1];
    uint8_t x = packet.payload.size() >= 3 ? packet.payload[2] : 0;
    uint8_t y = packet.payload.size() >= 4 ? packet.payload[3] : 0;

    PlayerState state;
    state.id = id;
    state.x = x;
    state.y = y;
    state.addr = from;
    _players[id] = state;

    std::cout << "[UDP] Registered client id=" << id << " at " << static_cast<int>(x) << "," << static_cast<int>(y) << "\n";
}

void UDPGameServer::handleInput(const Packet &packet, const sockaddr_in &from)
{
    if (packet.payload.size() < 4) {
        std::cerr << "[UDP] INPUT payload too small\n";
        return;
    }
    int id = (packet.payload[0] << 8) | packet.payload[1];
    uint8_t x = packet.payload[2];
    uint8_t y = packet.payload[3];

    auto it = _players.find(id);
    if (it == _players.end()) {
        PlayerState state;
        state.id = id;
        state.addr = from;
        state.x = x;
        state.y = y;
        _players[id] = state;
    } else {
        it->second.x = x;
        it->second.y = y;
        it->second.addr = from;
    }
}

void UDPGameServer::handlePacket(const Packet &packet, const sockaddr_in &from)
{
    switch (packet.type) {
        case PacketType::HELLO_UDP:
            handleHello(packet, from);
            break;
        case PacketType::INPUT:
            handleInput(packet, from);
            break;
        default:
            break;
    }
}

void UDPGameServer::run()
{
    uint8_t buffer[UDP_BUFFER_SIZE]{};
    _lastSnapshotMs = nowMs();

    while (true) {
        ssize_t n = _socket.readByte(buffer, sizeof(buffer));
        if (n > 0) {
            try {
                Packet packet = Packet::deserialize(buffer, static_cast<size_t>(n));
                handlePacket(packet, _socket.getSenderAddr());
            } catch (const std::exception &e) {
                std::cerr << "[UDP] Failed to parse packet: " << e.what() << "\n";
            }
        }

        long long now = nowMs();
        if (now - _lastSnapshotMs >= _snapshotIntervalMs) {
            broadcastSnapshot();
            _lastSnapshotMs = now;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
