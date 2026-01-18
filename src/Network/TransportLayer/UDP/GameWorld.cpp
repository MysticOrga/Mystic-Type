/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** Game world state and simulation (server authoritative)
*/

#include "GameWorld.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <limits>
#include <random>

namespace {
    constexpr float monsterHalf = 9.0f;  // ~18x18 in client
    constexpr float bossHalf = 22.0f;
    constexpr float bulletHalf = 3.0f;   // ~6x6 in client
    constexpr float playerHalfX = 16.5f; // ~33x17 in client
    constexpr float playerHalfY = 8.5f;
    constexpr bool kLogSpawn = false;
    constexpr bool kLogBullets = false;
    constexpr uint8_t kDefaultPlayerHp = 5;
    constexpr long long kPlayerHitCooldownMs = 500;
    constexpr int kKillScore = 10;
    constexpr uint16_t kBossScoreThreshold = 10;
    constexpr int8_t kBossHp = 5;
    constexpr int8_t kBossBulletMinVx = -12;
    constexpr int8_t kBossBulletMaxVx = -6;
    constexpr int8_t kBossBulletMaxVy = 6;
}

void GameWorld::registerPlayer(int id, uint8_t x, uint8_t y, const sockaddr_in &addr)
{
    PlayerState state;
    state.id = id;
    state.x = std::clamp<uint8_t>(x, 0, 255);
    state.y = std::clamp<uint8_t>(y, 0, 255);
    state.addr = addr;
    state.velX = 0;
    state.velY = 0;
    state.hp = kDefaultPlayerHp;
    state.score = _lobbyScore;
    state.lastHitMs = 0;
    _players[id] = state;
    _hadPlayers = true;
}

void GameWorld::updateInput(int id, int8_t velX, int8_t velY, uint8_t dir, const sockaddr_in &addr)
{
    auto it = _players.find(id);
    if (it == _players.end())
        return;

    PlayerState &p = it->second;
    p.addr = addr;
    constexpr int8_t maxSpeed = 10;
    p.velX = std::clamp<int8_t>(velX, -maxSpeed, maxSpeed);
    p.velY = std::clamp<int8_t>(velY, -maxSpeed, maxSpeed);
    p.dir = dir;
}

void GameWorld::addShot(int id, uint8_t posX, uint8_t posY, int8_t velX, int8_t velY)
{
    if (_players.find(id) == _players.end())
        return; // ignore shot from unknown player

    BulletState b;
    b.id = (_nextBulletId++ & 0xFFFF);
    b.ownerId = id;
    b.x = std::clamp<uint8_t>(posX, 0, 255);
    b.y = std::clamp<uint8_t>(posY, 0, 255);
    constexpr int8_t maxSpeed = 10;
    b.velX = std::clamp<int8_t>(velX, -maxSpeed, maxSpeed);
    b.velY = std::clamp<int8_t>(velY, -maxSpeed, maxSpeed);
    _bullets.push_back(b);

    const std::string prefix = _logPrefix.empty() ? "[UDP] " : _logPrefix;
    std::cout << prefix << "Player " << id << " fired bullet " << b.id
              << " from " << static_cast<int>(posX) << "," << static_cast<int>(posY)
              << " vel " << static_cast<int>(velX) << "," << static_cast<int>(velY) << "\n";
}

void GameWorld::removePlayer(int id)
{
    _players.erase(id);
    auto it = _bullets.begin();
    while (it != _bullets.end()) {
        if (it->ownerId == id) {
            it = _bullets.erase(it);
        } else {
            ++it;
        }
    }
    if (_hadPlayers && _players.empty()) {
        _noPlayersFlag = true;
    }
}

void GameWorld::spawnMonster(long long nowMs)
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
    int t = typeDist(rng);
    m.kind = t == 0 ? MonsterKind::Sine : MonsterKind::ZigZag;

    if (m.kind == MonsterKind::Sine) {
        m.amplitude = static_cast<float>(ampDist(rng));
        m.phase = 0.0f;
        m.freq = freqDist(rng);
        m.speedX = -1.3f;
    } else { // ZigZag
        m.amplitude = 22.0f;
        m.phase = 0.0f;
        m.freq = 0.0f;
        m.speedX = -1.4f;
    }
    _monsters.push_back(m);

    _monsterSpawnIntervalMs = intervalDist(rng);
    _lastMonsterSpawnMs = nowMs;
    if (kLogSpawn) {
        const std::string prefix = _logPrefix.empty() ? "[UDP] " : _logPrefix;
        std::cout << prefix << "Spawned monster " << m.id << " at y=" << m.baseY << " kind=" << static_cast<int>(m.kind) << "\n";
    }
}

bool GameWorld::shouldSpawnBoss() const
{
    return !_bossSpawnedOnce && _lobbyScore >= kBossScoreThreshold;
}

bool GameWorld::hasBoss() const
{
    for (const auto &m : _monsters) {
        if (m.kind == MonsterKind::Boss)
            return true;
    }
    return false;
}

void GameWorld::spawnBoss(long long nowMs)
{
    MonsterState m;
    m.id = (_nextMonsterId++ & 0xFFFF);
    m.x = 220.0f;
    m.y = 120.0f;
    m.baseY = m.y;
    m.hp = kBossHp;
    m.kind = MonsterKind::Boss;
    m.speedX = -0.8f;
    m.speedY = 0.6f;
    m.nextPatternMs = nowMs;
    m.nextShotMs = nowMs;
    _monsters.push_back(m);
    _bossSpawnedFlag = true;
    _bossSpawnedOnce = true;

    const std::string prefix = _logPrefix.empty() ? "[UDP] " : _logPrefix;
    std::cout << prefix << "Spawned BOSS " << m.id << "\n";
}

bool GameWorld::takeBossSpawned()
{
    bool wasSpawned = _bossSpawnedFlag;
    _bossSpawnedFlag = false;
    return wasSpawned;
}

bool GameWorld::takeBossDefeated()
{
    bool wasDefeated = _bossDefeatedFlag;
    _bossDefeatedFlag = false;
    return wasDefeated;
}

bool GameWorld::takeNoPlayers()
{
    bool wasEmpty = _noPlayersFlag;
    _noPlayersFlag = false;
    return wasEmpty;
}

void GameWorld::spawnBossBullet(const MonsterState &boss, long long nowMs)
{
    static std::mt19937 rng(static_cast<unsigned long>(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::uniform_int_distribution<int> vxDist(kBossBulletMinVx, kBossBulletMaxVx);
    std::uniform_int_distribution<int> vyDist(-kBossBulletMaxVy, kBossBulletMaxVy);

    BulletState b;
    b.id = (_nextBulletId++ & 0xFFFF);
    b.ownerId = -boss.id;
    b.x = static_cast<uint8_t>(std::clamp<int>(static_cast<int>(boss.x), 0, 255));
    b.y = static_cast<uint8_t>(std::clamp<int>(static_cast<int>(boss.y), 0, 255));
    b.velX = static_cast<int8_t>(vxDist(rng));
    b.velY = static_cast<int8_t>(vyDist(rng));
    _bullets.push_back(b);
    (void)nowMs;
}

void GameWorld::updateBossMovement(MonsterState &boss, long long nowMs, float dtSec)
{
    static std::mt19937 rng(static_cast<unsigned long>(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::uniform_real_distribution<float> speedDist(-2.2f, 2.2f);
    std::uniform_int_distribution<int> intervalDist(800, 1600);

    if (nowMs >= boss.nextPatternMs) {
        boss.speedX = speedDist(rng);
        boss.speedY = speedDist(rng) * 0.7f;
        boss.nextPatternMs = nowMs + intervalDist(rng);
    }

    boss.x += boss.speedX * dtSec * 32.0f;
    boss.y += boss.speedY * dtSec * 32.0f;

    constexpr float minX = 110.0f;
    constexpr float maxX = 245.0f;
    constexpr float minY = 20.0f;
    constexpr float maxY = 235.0f;
    if (boss.x < minX) {
        boss.x = minX;
        boss.speedX = std::fabs(boss.speedX);
    } else if (boss.x > maxX) {
        boss.x = maxX;
        boss.speedX = -std::fabs(boss.speedX);
    }
    if (boss.y < minY) {
        boss.y = minY;
        boss.speedY = std::fabs(boss.speedY);
    } else if (boss.y > maxY) {
        boss.y = maxY;
        boss.speedY = -std::fabs(boss.speedY);
    }
}

void GameWorld::tick(long long nowMs, long long deltaMs)
{
    bool bossActive = hasBoss();
    bool bossWanted = shouldSpawnBoss();
    if (bossWanted && !bossActive) {
        spawnBoss(nowMs);
        bossActive = true;
    }

    if (!bossActive && nowMs - _lastMonsterSpawnMs >= _monsterSpawnIntervalMs) {
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
        p.velX = 0;
        p.velY = 0;
    }

    for (auto &kv : _players) {
        auto &p = kv.second;
        if (p.hp == 0)
            continue;
        for (const auto &m : _monsters) {
            float half = (m.kind == MonsterKind::Boss) ? bossHalf : monsterHalf;
            float dx = std::fabs(m.x - static_cast<float>(p.x));
            float dy = std::fabs(m.y - static_cast<float>(p.y));
            if (dx <= half + playerHalfX && dy <= half + playerHalfY) {
                if (nowMs - p.lastHitMs >= kPlayerHitCooldownMs) {
                    int newHp = std::max(0, static_cast<int>(p.hp) - 1);
                    p.hp = static_cast<uint8_t>(newHp);
                    p.lastHitMs = nowMs;
                }
                break;
            }
        }
    }

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

    std::vector<bool> eraseBullet(_bullets.size(), false);
    for (std::size_t bi = 0; bi < _bullets.size(); ++bi) {
        const auto &b = _bullets[bi];
        if (b.ownerId >= 0)
            continue;
        for (auto &kv : _players) {
            auto &p = kv.second;
            if (p.hp == 0)
                continue;
            float dx = std::fabs(static_cast<float>(b.x) - static_cast<float>(p.x));
            float dy = std::fabs(static_cast<float>(b.y) - static_cast<float>(p.y));
            if (dx <= bulletHalf + playerHalfX && dy <= bulletHalf + playerHalfY) {
                if (nowMs - p.lastHitMs >= kPlayerHitCooldownMs) {
                    int newHp = std::max(0, static_cast<int>(p.hp) - 1);
                    p.hp = static_cast<uint8_t>(newHp);
                    p.lastHitMs = nowMs;
                }
                eraseBullet[bi] = true;
                break;
            }
        }
    }

    std::vector<int> bulletsToErase;
    for (std::size_t bi = 0; bi < _bullets.size(); ++bi) {
        if (eraseBullet[bi])
            continue;
        const auto &b = _bullets[bi];
        if (b.ownerId < 0)
            continue;
        bool hit = false;
        for (auto &m : _monsters) {
            if (m.hp <= 0)
                continue;
            float half = (m.kind == MonsterKind::Boss) ? bossHalf : monsterHalf;
            float dx = std::fabs(m.x - static_cast<float>(b.x));
            float dy = std::fabs(m.y - static_cast<float>(b.y));
            if (dx <= half + bulletHalf && dy <= half + bulletHalf) {
                m.hp = static_cast<int8_t>(m.hp - 1);
                if (m.hp <= 0) {
                    int maxScore = std::numeric_limits<uint16_t>::max();
                    int newScore = std::min<int>(_lobbyScore + kKillScore, maxScore);
                    _lobbyScore = static_cast<uint16_t>(newScore);
                }
                hit = true;
                break;
            }
        }
        if (hit) {
            bulletsToErase.push_back(static_cast<int>(bi));
            _monsterKilled += 1;
            std::cerr << "Monster killed: " << (int)_monsterKilled << std::endl;
        }
    }
    for (std::size_t bi = 0; bi < eraseBullet.size(); ++bi) {
        if (eraseBullet[bi])
            bulletsToErase.push_back(static_cast<int>(bi));
    }
    std::sort(bulletsToErase.rbegin(), bulletsToErase.rend());
    for (int idx : bulletsToErase) {
        if (idx >= 0 && static_cast<std::size_t>(idx) < _bullets.size())
            _bullets.erase(_bullets.begin() + idx);
    }

    auto mIt = _monsters.begin();
    while (mIt != _monsters.end()) {
        if (mIt->hp <= 0) {
            if (mIt->kind == MonsterKind::Boss) {
                _bossDefeatedFlag = true;
            }
            mIt = _monsters.erase(mIt);
        } else {
            ++mIt;
        }
    }

    float dtSec = static_cast<float>(deltaMs) / 1000.0f;
    auto mit = _monsters.begin();
    while (mit != _monsters.end()) {
        if (mit->kind == MonsterKind::Boss) {
            updateBossMovement(*mit, nowMs, dtSec);
            if (nowMs >= mit->nextShotMs) {
                static std::mt19937 rng(static_cast<unsigned long>(std::chrono::steady_clock::now().time_since_epoch().count()));
                std::uniform_int_distribution<int> intervalDist(350, 700);
                spawnBossBullet(*mit, nowMs);
                mit->nextShotMs = nowMs + intervalDist(rng);
            }
            ++mit;
            continue;
        }

        mit->phase += mit->freq * dtSec;
        mit->x += mit->speedX * dtSec * 32.0f;
        float oscillation = 0.0f;
        if (mit->kind == MonsterKind::Sine) {
            oscillation = std::sin(mit->phase);
        } else { // ZigZag: alternate up/down every ~0.4s
            float period = 0.4f;
            float phaseT = std::fmod(static_cast<float>(nowMs) / 1000.0f, period * 2.0f);
            oscillation = (phaseT < period) ? 1.0f : -1.0f;
        }
        mit->y = mit->baseY + mit->amplitude * oscillation;
        if (mit->x < -5.0f || mit->y < -5.0f || mit->y > 260.0f) {
            mit = _monsters.erase(mit);
        } else {
            ++mit;
        }
    }

    if (kLogBullets && !_bullets.empty()) {
        const std::string prefix = _logPrefix.empty() ? "[UDP] " : _logPrefix;
        std::cout << prefix << "Bullets: ";
        for (const auto &b : _bullets) {
            std::cout << b.id << "(" << static_cast<int>(b.x) << "," << static_cast<int>(b.y) << ") ";
        }
        std::cout << "\n";
    }
}

Packet GameWorld::buildSnapshotPacket()
{
    std::vector<uint8_t> payload;
    payload.reserve(2 + _players.size() * 7 + _bullets.size() * 6 + _monsters.size() * 6);

    payload.push_back(static_cast<uint8_t>(_players.size()));
    for (const auto &kv : _players) {
        const auto &p = kv.second;
        payload.push_back(static_cast<uint8_t>((p.id >> 8) & 0xFF));
        payload.push_back(static_cast<uint8_t>(p.id & 0xFF));
        payload.push_back(p.x);
        payload.push_back(p.y);
        payload.push_back(p.hp);
        payload.push_back(static_cast<uint8_t>((_lobbyScore >> 8) & 0xFF));
        payload.push_back(static_cast<uint8_t>(_lobbyScore & 0xFF));
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
    _snapshotSeq = static_cast<uint16_t>(_snapshotSeq + 1);
    payload.push_back(static_cast<uint8_t>((_snapshotSeq >> 8) & 0xFF));
    payload.push_back(static_cast<uint8_t>(_snapshotSeq & 0xFF));
    return Packet(PacketType::SNAPSHOT, payload);
}
