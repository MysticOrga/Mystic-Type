/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** UDP Game Server
*/

#include "UDPGameServer.hpp"
#include <iostream>
#include <thread>
#include <sstream>

namespace {
    constexpr std::size_t UDP_BUFFER_SIZE = 1024;
}

UDPGameServer::UDPGameServer(uint16_t port, SessionManager &sessions, long long snapshotIntervalMs, std::string lobbyCode)
    : _port(port), _snapshotIntervalMs(snapshotIntervalMs), _sessions(sessions), _expectedLobby(std::move(lobbyCode))
{
    if (!_socket.bindTo(port)) {
        throw std::runtime_error("Failed to bind UDP socket");
    }
    std::cout << logPrefix() << "Listening on port " << port << std::endl;
    if (!_expectedLobby.empty()) {
        _worlds[_expectedLobby] = GameWorld{};
        _worlds[_expectedLobby].setLogPrefix(logPrefix());
    }
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
        std::cerr << logPrefix() << "HELLO_UDP payload too small\n";
        return;
    }
    int id = (packet.payload[0] << 8) | packet.payload[1];
    uint8_t x = packet.payload.size() >= 3 ? packet.payload[2] : 0;
    uint8_t y = packet.payload.size() >= 4 ? packet.payload[3] : 0;

    // Use expected lobby when provided; fallback to session info or PUBLIC.
    std::string lobbyCode = !_expectedLobby.empty() ? _expectedLobby : "PUBLIC";
    auto sessionOpt = _sessions.getSession(id);
    if (sessionOpt.has_value() && !sessionOpt->lobbyCode.empty()) {
        lobbyCode = sessionOpt->lobbyCode;
        _sessions.setUdpAddr(id, from);
    }

    GameWorld &world = _worlds[lobbyCode];
    world.setLogPrefix(logPrefix());
    world.registerPlayer(id, x, y, from);
    _playerLobby[id] = lobbyCode;
    // Send a fresh snapshot immediately so the client sees the lobby state without waiting the next tick.
    Packet snap = world.buildSnapshotPacket();
    sendPacketTo(snap, from);
    std::cout << logPrefix() << "Registered client id=" << id << " at " << static_cast<int>(x) << "," << static_cast<int>(y) << "\n";
}

void UDPGameServer::handleInput(const Packet &packet, const sockaddr_in &from)
{
    if (packet.payload.size() < 7) {
        std::cerr << logPrefix() << "INPUT payload too small\n";
        return;
    }
    int id = (packet.payload[0] << 8) | packet.payload[1];
    // ignore client-provided position to keep authority
    int8_t velX = static_cast<int8_t>(packet.payload[4]);
    int8_t velY = static_cast<int8_t>(packet.payload[5]);
    uint8_t dir = packet.payload[6];

    // Rate limiting disabled here because SessionManager is not shared across processes.
    auto lobbyIt = _playerLobby.find(id);
    if (lobbyIt == _playerLobby.end()) {
        if (!_expectedLobby.empty()) {
            _playerLobby[id] = _expectedLobby;
        } else {
            auto sessionOpt = _sessions.getSession(id);
            if (sessionOpt.has_value() && !sessionOpt->lobbyCode.empty()) {
                _playerLobby[id] = sessionOpt->lobbyCode;
            } else {
                _playerLobby[id] = "PUBLIC";
            }
        }
        lobbyIt = _playerLobby.find(id);
    }

    auto worldIt = _worlds.find(lobbyIt->second);
    if (worldIt == _worlds.end()) {
        worldIt = _worlds.emplace(lobbyIt->second, GameWorld{}).first;
    }
    worldIt->second.setLogPrefix(logPrefix());

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

    auto lobbyIt = _playerLobby.find(id);
    if (lobbyIt == _playerLobby.end()) {
        if (!_expectedLobby.empty()) {
            _playerLobby[id] = _expectedLobby;
        } else {
            auto sessionOpt = _sessions.getSession(id);
            if (sessionOpt.has_value() && !sessionOpt->lobbyCode.empty()) {
                _playerLobby[id] = sessionOpt->lobbyCode;
            } else {
                _playerLobby[id] = "PUBLIC";
            }
        }
        lobbyIt = _playerLobby.find(id);
    }
    auto worldIt = _worlds.find(lobbyIt->second);
    if (worldIt == _worlds.end()) {
        worldIt = _worlds.emplace(lobbyIt->second, GameWorld{}).first;
    }
    worldIt->second.setLogPrefix(logPrefix());

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
                std::cerr << logPrefix() << "Failed to parse packet: " << e.what() << "\n";
            }
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}

std::string UDPGameServer::logPrefix() const
{
    std::ostringstream oss;
    oss << "[UDP lobby=" << (_expectedLobby.empty() ? "?" : _expectedLobby)
        << " port=" << _port << " tid=" << std::this_thread::get_id() << "] ";
    return oss.str();
}
