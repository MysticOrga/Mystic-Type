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

UDPGameServer::UDPGameServer(uint16_t port, SessionManager &sessions, long long snapshotIntervalMs)
    : _port(port), _snapshotIntervalMs(snapshotIntervalMs), _sessions(sessions)
{
    if (!_socket.bindTo(port)) {
        throw std::runtime_error("Failed to bind UDP socket");
    }
    std::cout << "[UDP] Listening on port " << port << std::endl;
    _sessions.setOnRemove([this](int id) {
        auto itLobby = _playerLobby.find(id);
        if (itLobby != _playerLobby.end()) {
            auto worldIt = _worlds.find(itLobby->second);
            if (worldIt != _worlds.end()) {
                worldIt->second.removePlayer(id);
                if (worldIt->second.players().empty()) {
                    _worlds.erase(worldIt);
                }
            }
            _playerLobby.erase(itLobby);
        }
    });
}

UDPGameServer::~UDPGameServer()
{
    _running = false;
    if (_networkThread.joinable()) {
        _networkThread.join();
    }
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
    for (auto &kv : _worlds) {
        Packet snap = kv.second.buildSnapshotPacket();
        for (const auto &player : kv.second.players()) {
            sendPacketTo(snap, player.second.addr);
        }
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

    auto sessionOpt = _sessions.getSession(id);
    if (!sessionOpt.has_value()) {
        std::cerr << "[UDP] HELLO_UDP from unknown id=" << id << "\n";
        return;
    }
    if (sessionOpt->lobbyCode.empty()) {
        std::cerr << "[UDP] HELLO_UDP without lobby code for id=" << id << "\n";
        return;
    }

    GameWorld &world = _worlds[sessionOpt->lobbyCode];
    world.registerPlayer(id, x, y, from);
    _sessions.setUdpAddr(id, from);
    _playerLobby[id] = sessionOpt->lobbyCode;
    // Send a fresh snapshot immediately so the client sees the lobby state without waiting the next tick.
    Packet snap = world.buildSnapshotPacket();
    sendPacketTo(snap, from);
    std::cout << "[UDP] Registered client id=" << id << " at " << static_cast<int>(x) << "," << static_cast<int>(y) << "\n";
}

void UDPGameServer::handleInput(const Packet &packet, const sockaddr_in &from)
{
    if (packet.payload.size() < 7) {
        std::cerr << "[UDP] INPUT payload too small\n";
        return;
    }
    int id = (packet.payload[0] << 8) | packet.payload[1];
    // ignore client-provided position to keep authority
    int8_t velX = static_cast<int8_t>(packet.payload[4]);
    int8_t velY = static_cast<int8_t>(packet.payload[5]);
    uint8_t dir = packet.payload[6];

    long now = nowMs();
    if (!_sessions.allowInput(id, now)) {
        std::cerr << "[UDP] INPUT rate limited for id=" << id << "\n";
        return;
    }
    auto lobbyIt = _playerLobby.find(id);
    if (lobbyIt == _playerLobby.end()) {
        auto sessionOpt = _sessions.getSession(id);
        if (!sessionOpt.has_value() || sessionOpt->lobbyCode.empty()) {
            std::cerr << "[UDP] INPUT from unknown lobby id=" << id << "\n";
            return;
        }
        _playerLobby[id] = sessionOpt->lobbyCode;
        lobbyIt = _playerLobby.find(id);
    }

    auto worldIt = _worlds.find(lobbyIt->second);
    if (worldIt == _worlds.end()) {
        std::cerr << "[UDP] INPUT with missing world for lobby " << lobbyIt->second << "\n";
        return;
    }

    worldIt->second.updateInput(id, velX, velY, dir, from);
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

    long now = nowMs();
    if (!_sessions.allowShoot(id, now)) {
        std::cerr << "[UDP] SHOOT rate limited for id=" << id << "\n";
        return;
    }
    auto lobbyIt = _playerLobby.find(id);
    if (lobbyIt == _playerLobby.end()) {
        auto sessionOpt = _sessions.getSession(id);
        if (!sessionOpt.has_value() || sessionOpt->lobbyCode.empty())
            return;
        _playerLobby[id] = sessionOpt->lobbyCode;
        lobbyIt = _playerLobby.find(id);
    }
    auto worldIt = _worlds.find(lobbyIt->second);
    if (worldIt == _worlds.end())
        return;

    worldIt->second.addShot(id, posX, posY, velX, velY);
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
    _running = true;
    _networkThread = std::thread(&UDPGameServer::networkLoop, this);

    _lastSnapshotMs = nowMs();
    _lastTickMs = _lastSnapshotMs;

    while (_running) {
        {
            std::lock_guard<std::mutex> lock(_queueMutex);
            while (!_incoming.empty()) {
                auto item = _incoming.front();
                _incoming.pop();
                handlePacket(item.pkt, item.from);
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

    if (_networkThread.joinable()) {
        _networkThread.join();
    }
}

void UDPGameServer::updateSimulation(long long nowMs, long long deltaMs)
{
    for (auto &kv : _worlds) {
        kv.second.tick(nowMs, deltaMs);
    }
}

void UDPGameServer::networkLoop()
{
    uint8_t buffer[UDP_BUFFER_SIZE]{};
    while (_running) {
        ssize_t n = _socket.readByte(buffer, sizeof(buffer));
        if (n > 0) {
            try {
                Packet packet = Packet::deserialize(buffer, static_cast<size_t>(n));
                Incoming inc;
                inc.pkt = std::move(packet);
                inc.from = _socket.getSenderAddr();
                std::lock_guard<std::mutex> lock(_queueMutex);
                _incoming.push(std::move(inc));
            } catch (const std::exception &e) {
                std::cerr << "[UDP] Failed to parse packet: " << e.what() << "\n";
            }
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}
