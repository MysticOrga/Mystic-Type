/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** UDP Game Server
*/

#include "UDPGameServer.hpp"
#include <iostream>
#include <thread>
#include <algorithm>
#include <random>
#include <cmath>

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
    payload.reserve(2 + _players.size() * 4 + _bullets.size() * 6 + _monsters.size() * 6);

    payload.push_back(static_cast<uint8_t>(_players.size()));
    for (const auto &kv : _players) {
        const auto &p = kv.second;
        payload.push_back(static_cast<uint8_t>((p.id >> 8) & 0xFF));
        payload.push_back(static_cast<uint8_t>(p.id & 0xFF));
        payload.push_back(p.x);
        payload.push_back(p.y);
    }
    payload.push_back(static_cast<uint8_t>(_bullets.size()));
    for (const auto &b : _bullets) {
        payload.push_back(static_cast<uint8_t>((b.id >> 8) & 0xFF));
        payload.push_back(static_cast<uint8_t>(b.id & 0xFF));
        payload.push_back(b.x);
        payload.push_back(b.y);
        payload.push_back(static_cast<uint8_t>(b.velX));
        payload.push_back(static_cast<uint8_t>(b.velY));
    }
    payload.push_back(static_cast<uint8_t>(_monsters.size()));
    for (const auto &m : _monsters) {
        payload.push_back(static_cast<uint8_t>((m.id >> 8) & 0xFF));
        payload.push_back(static_cast<uint8_t>(m.id & 0xFF));
        payload.push_back(static_cast<uint8_t>(std::clamp<int>(static_cast<int>(m.x), 0, 255)));
        payload.push_back(static_cast<uint8_t>(std::clamp<int>(static_cast<int>(m.y), 0, 255)));
        payload.push_back(static_cast<uint8_t>(std::clamp<int>(m.hp, 0, 127)));
        payload.push_back(static_cast<uint8_t>(m.kind));
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
    state.velX = 0;
    state.velY = 0;
    _players[id] = state;

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

    auto it = _players.find(id);
    // Si le joueur n'existe plus, ignore l'input pour éviter de le recréer
    if (it == _players.end())
        return;

    PlayerState &p = it->second;
    p.addr = from;
    p.x = posX;
    p.y = posY;
    p.velX = velX;
    p.velY = velY;
    p.dir = dir;
}

void UDPGameServer::handleShoot(const Packet &packet)
{
    if (packet.payload.size() < 6)
        return;

    int id = (packet.payload[0] << 8) | packet.payload[1];
    if (_players.find(id) == _players.end())
        return; // ignore tir d'un joueur inexistant (mort)
    uint8_t posX = packet.payload[2];
    uint8_t posY = packet.payload[3];
    int8_t velX = static_cast<int8_t>(packet.payload[4]);
    int8_t velY = static_cast<int8_t>(packet.payload[5]);

    BulletState b;
    b.id = (_nextBulletId++ & 0xFFFF);
    b.x = posX;
    b.y = posY;
    b.velX = velX;
    b.velY = velY;
    _bullets.push_back(b);

    std::cout << "[UDP] Player " << id << " fired bullet " << b.id
              << " from " << static_cast<int>(posX) << "," << static_cast<int>(posY)
              << " vel " << static_cast<int>(velX) << "," << static_cast<int>(velY) << "\n";
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
    _lastMonsterSpawnMs = _lastSnapshotMs;

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

void UDPGameServer::spawnMonster(long long nowMs)
{
    static std::mt19937 rng(static_cast<unsigned long>(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::uniform_int_distribution<int> yDist(20, 235);
    std::uniform_int_distribution<int> ampDist(8, 18);
    std::uniform_real_distribution<float> freqDist(2.5f, 5.0f);
    std::uniform_int_distribution<int> intervalDist(1600, 2400);
    std::uniform_int_distribution<int> typeDist(0, 1);

    MonsterState m;
    m.id = (_nextMonsterId++ & 0xFFFF);
    m.x = 255.0f;
    m.baseY = static_cast<float>(yDist(rng));
    m.y = m.baseY;
    m.hp = 3;
    m.kind = typeDist(rng) == 0 ? MonsterKind::Sine : MonsterKind::Cosine;

    // Paramètres distincts selon le type pour un pattern visuellement différent
    if (m.kind == MonsterKind::Sine) {
        m.amplitude = static_cast<float>(ampDist(rng));
        m.phase = 0.0f;
        m.freq = freqDist(rng);      // 2.5 -> 5
        m.speedX = -1.3f;
    } else {
        // Cosinus : amplitude un peu plus grande, fréquence plus lente, phase décalée
        std::uniform_int_distribution<int> ampDistCos(12, 24);
        std::uniform_real_distribution<float> freqDistCos(1.5f, 3.0f);
        m.amplitude = static_cast<float>(ampDistCos(rng));
        m.phase = static_cast<float>(M_PI_2); // décalage de phase
        m.freq = freqDistCos(rng);
        m.speedX = -1.1f; // léger différentiel de vitesse
    }
    _monsters.push_back(m);

    _monsterSpawnIntervalMs = intervalDist(rng);
    _lastMonsterSpawnMs = nowMs;
    std::cout << "[UDP] Spawned monster " << m.id << " at y=" << m.baseY << "\n";
}

void UDPGameServer::updateSimulation(long long nowMs, long long deltaMs)
{
    if (nowMs - _lastMonsterSpawnMs >= _monsterSpawnIntervalMs) {
        spawnMonster(nowMs);
    }

    for (auto &kv : _players) {
        auto &p = kv.second;
        int nx = static_cast<int>(p.x) + p.velX;
        int ny = static_cast<int>(p.y) + p.velY;
        nx = std::clamp(nx, 0, 255);
        ny = std::clamp(ny, 0, 255);
        p.x = static_cast<uint8_t>(nx);
        p.y = static_cast<uint8_t>(ny);
        // Reset velocity after applying one-step movement
        p.velX = 0;
        p.velY = 0;
    }

    // Move bullets and remove those that leave the arena
    auto it = _bullets.begin();
    while (it != _bullets.end()) {
        int nx = static_cast<int>(it->x) + it->velX;
        int ny = static_cast<int>(it->y) + it->velY;
        if (nx < 0 || nx > 255 || ny < 0 || ny > 255) {
            it = _bullets.erase(it);
            continue;
        }
        it->x = static_cast<uint8_t>(nx);
        it->y = static_cast<uint8_t>(ny);
        ++it;
    }

    // Bullet/monster collisions
    const float monsterHalf = 9.0f; // ~18x18 in client
    const float bulletHalf = 3.0f;  // ~6x6 in client
    std::vector<int> bulletsToErase;
    for (std::size_t bi = 0; bi < _bullets.size(); ++bi) {
        const auto &b = _bullets[bi];
        bool hit = false;
        for (auto &m : _monsters) {
            float dx = std::fabs(m.x - static_cast<float>(b.x));
            float dy = std::fabs(m.y - static_cast<float>(b.y));
            if (dx <= monsterHalf + bulletHalf && dy <= monsterHalf + bulletHalf) {
                m.hp -= 1;
                hit = true;
                break;
            }
        }
        if (hit)
            bulletsToErase.push_back(static_cast<int>(bi));
    }
    // remove bullets hit (from back to front)
    std::sort(bulletsToErase.rbegin(), bulletsToErase.rend());
    for (int idx : bulletsToErase) {
        if (idx >= 0 && static_cast<std::size_t>(idx) < _bullets.size())
            _bullets.erase(_bullets.begin() + idx);
    }
    // remove dead monsters
    auto mIt = _monsters.begin();
    while (mIt != _monsters.end()) {
        if (mIt->hp <= 0) {
            mIt = _monsters.erase(mIt);
        } else {
            ++mIt;
        }
    }

    // Update monsters with sinus pattern
    float dtSec = static_cast<float>(deltaMs) / 1000.0f;
    auto mit = _monsters.begin();
    while (mit != _monsters.end()) {
        mit->phase += mit->freq * dtSec;
        mit->x += mit->speedX * dtSec * 32.0f; // scale to roughly match player units per tick
        float oscillation = (mit->kind == MonsterKind::Sine)
            ? std::sin(mit->phase)
            : std::cos(mit->phase);
        mit->y = mit->baseY + mit->amplitude * oscillation;
        if (mit->x < -5.0f || mit->y < -5.0f || mit->y > 260.0f) {
            mit = _monsters.erase(mit);
        } else {
            ++mit;
        }
    }

    // Plus de gestion de collision joueurs/monstres ni de PV joueurs

    if (!_bullets.empty()) {
        std::cout << "[UDP] Bullets: ";
        for (const auto &b : _bullets) {
            std::cout << b.id << "(" << static_cast<int>(b.x) << "," << static_cast<int>(b.y) << ") ";
        }
        std::cout << "\n";
    }
}
