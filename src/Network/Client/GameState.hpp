/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** Game state for client
*/

#pragma once

#include <unordered_map>
#include <vector>
#include <cstdint>

/**
 * @brief Lightweight snapshot of a player for client-side rendering.
 */
struct PlayerState {
    int id = 0;
    uint8_t x = 0;
    uint8_t y = 0;
    uint8_t hp = 0;
    uint16_t score = 0;
};

/**
 * @brief Lightweight snapshot of a bullet for client-side rendering.
 */
struct BulletState {
    int id = 0;
    uint8_t x = 0;
    uint8_t y = 0;
    int8_t vx = 0;
    int8_t vy = 0;
};

/**
 * @brief Lightweight snapshot of a monster for client-side rendering.
 */
struct MonsterState {
    int id = 0;
    uint8_t x = 0;
    uint8_t y = 0;
    uint8_t hp = 0;
    uint8_t type = 0; // 0 = sin, 1 = cos
};

/**
 * @brief Client-side container for the latest world state.
 */
class GameState {
public:
    /**
     * @brief Insert or update a player entry.
     */
    void upsertPlayer(int id, uint8_t x, uint8_t y, uint8_t hp, uint16_t score)
    {
        _players[id] = PlayerState{id, x, y, hp, score};
    }

    /**
     * @brief Insert or update a bullet entry.
     */
    void upsertBullet(int id, uint8_t x, uint8_t y, int8_t vx, int8_t vy)
    {
        _bullets[id] = BulletState{id, x, y, vx, vy};
    }

    /**
     * @brief Insert or update a monster entry.
     */
    void upsertMonster(int id, uint8_t x, uint8_t y, uint8_t hp, uint8_t type)
    {
        _monsters[id] = MonsterState{id, x, y, hp, type};
    }

    /**
     * @brief Clear all cached entities.
     */
    void clear()
    {
        _players.clear();
        _bullets.clear();
        _monsters.clear();
    }

    /**
     * @brief Clear only player entries.
     */
    void clearPlayers()
    {
        _players.clear();
    }

    /**
     * @brief Return a snapshot list of current players.
     */
    std::vector<PlayerState> listPlayers() const
    {
        std::vector<PlayerState> res;
        res.reserve(_players.size());
        for (const auto &kv : _players)
            res.push_back(kv.second);
        return res;
    }

    /**
     * @brief Return a snapshot list of current bullets.
     */
    std::vector<BulletState> listBullets() const
    {
        std::vector<BulletState> res;
        res.reserve(_bullets.size());
        for (const auto &kv : _bullets)
            res.push_back(kv.second);
        return res;
    }

    /**
     * @brief Return a snapshot list of current monsters.
     */
    std::vector<MonsterState> listMonsters() const
    {
        std::vector<MonsterState> res;
        res.reserve(_monsters.size());
        for (const auto &kv : _monsters)
            res.push_back(kv.second);
        return res;
    }

private:
    std::unordered_map<int, PlayerState> _players;
    std::unordered_map<int, BulletState> _bullets;
    std::unordered_map<int, MonsterState> _monsters;
};
