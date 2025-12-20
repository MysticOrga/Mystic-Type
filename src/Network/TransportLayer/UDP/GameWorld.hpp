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
    #include <winsock2.h>
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
    struct PlayerState {
        int id = 0;
        uint8_t x = 0;
        uint8_t y = 0;
        sockaddr_in addr{};
        int8_t velX = 0;
        int8_t velY = 0;
        uint8_t dir = 0;
    };

        struct BulletState {
            int id = 0;
            int ownerId = 0;
            uint8_t x = 0;
            uint8_t y = 0;
            int8_t velX = 0;
            int8_t velY = 0;
        };

    enum class MonsterKind : uint8_t { Sine = 0, Cosine = 1 };

    struct MonsterState {
        int id = 0;
        float x = 0;
        float y = 0;
        float baseY = 0;
        float amplitude = 0;
        float phase = 0;
        float freq = 0;
        float speedX = 0;
        int8_t hp = 0;
        MonsterKind kind = MonsterKind::Sine;
    };

    GameWorld() = default;

    void registerPlayer(int id, uint8_t x, uint8_t y, const sockaddr_in &addr);
        void updateInput(int id, int8_t velX, int8_t velY, uint8_t dir, const sockaddr_in &addr);
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

    Packet buildSnapshotPacket() const;

    const std::unordered_map<int, PlayerState> &players() const { return _players; }

private:
    void spawnMonster(long long nowMs);

    std::unordered_map<int, PlayerState> _players;
    std::vector<BulletState> _bullets;
    std::vector<MonsterState> _monsters;
    int _nextBulletId = 1;
    int _nextMonsterId = 1;
    long long _lastMonsterSpawnMs = 0;
    long long _monsterSpawnIntervalMs = 1800;
    uint8_t _monsterKilled = 0;
};
