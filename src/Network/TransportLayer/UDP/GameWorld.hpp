/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** Game world state and simulation (server authoritative)
*/

#pragma once

#include "../Packet.hpp"
#ifndef _WIN32
    #include <netinet/in.h>
#else
#define NOMINMAX
    #include <winsock2.h>
    constexpr double PI = 3.14159265358979323846;
    constexpr double HALF_PI = PI / 2.0;

#endif
#include <unordered_map>
#include <vector>
#include <cstdint>

/**
 * @brief Server-side authoritative simulation for players, bullets and monsters.
 *
 * Keeps track of entity states and performs time-based updates (movement, spawn, collisions).
 * All networking code stays in UDPGameServer; this class only handles game logic/state.
 */
class GameWorld {
public:
    /**
     * @brief Server-side player state with last known UDP address.
     */
    struct PlayerState {
        int id = 0;
        uint8_t x = 0;
        uint8_t y = 0;
        uint8_t hp = 0;
        uint16_t score = 0;
        sockaddr_in addr{};
        int8_t velX = 0;
        int8_t velY = 0;
        uint8_t dir = 0;
        long long lastHitMs = 0;
    };

    /**
     * @brief Bullet state owned by a player.
     */
    struct BulletState {
            int id = 0;
            int ownerId = 0;
            uint8_t x = 0;
            uint8_t y = 0;
            int8_t velX = 0;
            int8_t velY = 0;
        };

    /**
     * @brief Monster movement/behavior type.
     */
    enum class MonsterKind : uint8_t { Sine = 0, ZigZag = 1, Boss = 2 };

    /**
     * @brief Monster state tracked by the authoritative server.
     */
    struct MonsterState {
        int id = 0;
        float x = 0;
        float y = 0;
        float baseY = 0;
        float amplitude = 0;
        float phase = 0;
        float freq = 0;
        float speedX = 0;
        float speedY = 0;
        int8_t hp = 0;
        MonsterKind kind = MonsterKind::Sine;
        long long nextPatternMs = 0;
        long long nextShotMs = 0;
    };

    GameWorld() = default;

    /**
     * @brief Register a player on HELLO_UDP.
     */
    void registerPlayer(int id, uint8_t x, uint8_t y, const sockaddr_in &addr);
    /**
     * @brief Update input state and refresh the sender address.
     */
    void updateInput(int id, int8_t velX, int8_t velY, uint8_t dir, const sockaddr_in &addr);
    /**
     * @brief Register a player shot in the world.
     */
    void addShot(int id, uint8_t posX, uint8_t posY, int8_t velX, int8_t velY);

        /**
         * @brief Remove a player and any references to it.
         */
        void removePlayer(int id);

        /**
         * @brief Advance simulation.
         * @param nowMs Current time in ms.
         * @param deltaMs Time since last tick in ms.
         */
    void tick(long long nowMs, long long deltaMs);

    /**
     * @brief Build a UDP snapshot packet of current world state.
     */
    Packet buildSnapshotPacket();
    /**
     * @brief Consume boss-spawned flag (one-shot).
     */
    bool takeBossSpawned();
    /**
     * @brief Consume boss-defeated flag (one-shot).
     */
    bool takeBossDefeated();
    /**
     * @brief Consume no-players flag (one-shot).
     */
    bool takeNoPlayers();
    /**
     * @brief True after at least one player has joined.
     */
    bool hasHadPlayers() const { return _hadPlayers; }

    /**
     * @brief Access player map (read-only).
     */
    const std::unordered_map<int, PlayerState> &players() const { return _players; }
    /**
     * @brief Prefix used in log output.
     */
    void setLogPrefix(const std::string &prefix) { _logPrefix = prefix; }

private:
    void spawnMonster(long long nowMs);
    void spawnBoss(long long nowMs);
    void spawnBossBullet(const MonsterState &boss, long long nowMs);
    bool shouldSpawnBoss() const;
    bool hasBoss() const;
    void updateBossMovement(MonsterState &boss, long long nowMs, float dtSec);

    std::unordered_map<int, PlayerState> _players;
    std::vector<BulletState> _bullets;
    std::vector<MonsterState> _monsters;
    size_t _monsterKilled = 0;
    int _nextBulletId = 1;
    int _nextMonsterId = 1;
    long long _lastMonsterSpawnMs = 0;
    long long _monsterSpawnIntervalMs = 1800;
    bool _bossSpawnedFlag = false;
    bool _bossSpawnedOnce = false;
    bool _bossDefeatedFlag = false;
    bool _hadPlayers = false;
    bool _noPlayersFlag = false;
    uint16_t _lobbyScore = 0;
    uint16_t _snapshotSeq = 0;
    std::string _logPrefix;
};
