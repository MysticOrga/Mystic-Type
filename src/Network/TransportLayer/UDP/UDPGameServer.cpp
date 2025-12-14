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

void UDPGameServer::broadcastSnapshot()
{
    Packet snap = _world.buildSnapshotPacket();
    for (const auto &kv : _world.players()) {
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

    _world.registerPlayer(id, x, y, from);
    std::cout << "[UDP] Registered client id=" << id << " at " << static_cast<int>(x) << "," << static_cast<int>(y) << "\n";
}

void UDPGameServer::handleInput(const Packet &packet, const sockaddr_in &from)
{
    if (packet.payload.size() < 7) {
        std::cerr << "[UDP] INPUT payload too small\n";
        return;
    }
    int id = (packet.payload[0] << 8) | packet.payload[1];
    uint8_t posX = packet.payload[2];
    uint8_t posY = packet.payload[3];
    int8_t velX = static_cast<int8_t>(packet.payload[4]);
    int8_t velY = static_cast<int8_t>(packet.payload[5]);
    uint8_t dir = packet.payload[6];

    _world.updateInput(id, posX, posY, velX, velY, dir, from);
}

void UDPGameServer::handleShoot(const Packet &packet)
{
    if (packet.payload.size() < 6)
        return;

    int id = (packet.payload[0] << 8) | packet.payload[1];
    uint8_t posX = packet.payload[2];
    uint8_t posY = packet.payload[3];
    int8_t velX = static_cast<int8_t>(packet.payload[4]);
    int8_t velY = static_cast<int8_t>(packet.payload[5]);

    _world.addShot(id, posX, posY, velX, velY);
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
        case PacketType::SHOOT:
            handleShoot(packet);
            break;
        default:
            break;
    }
}

void UDPGameServer::run()
{
    uint8_t buffer[UDP_BUFFER_SIZE]{};
    _lastSnapshotMs = nowMs();
    _lastTickMs = _lastSnapshotMs;

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
        if (now - _lastTickMs >= _tickIntervalMs) {
            updateSimulation(now, now - _lastTickMs);
            _lastTickMs = now;
        }

        if (now - _lastSnapshotMs >= _snapshotIntervalMs) {
            broadcastSnapshot();
            _lastSnapshotMs = now;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void UDPGameServer::updateSimulation(long long nowMs, long long deltaMs)
{
    _world.tick(nowMs, deltaMs);
}
