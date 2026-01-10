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

struct PlayerState {
    int id = 0;
    uint8_t x = 0;
    uint8_t y = 0;
    uint8_t hp = 0;
};

struct BulletState {
    int id = 0;
    uint8_t x = 0;
    uint8_t y = 0;
    int8_t vx = 0;
    int8_t vy = 0;
};

struct MonsterState {
    int id = 0;
    uint8_t x = 0;
    uint8_t y = 0;
    uint8_t hp = 0;
    uint8_t type = 0; // 0 = sin, 1 = cos
};

class GameState {
public:
    void upsertPlayer(int id, uint8_t x, uint8_t y, uint8_t hp)
    {
        _players[id] = PlayerState{id, x, y, hp};
    }

    void upsertBullet(int id, uint8_t x, uint8_t y, int8_t vx, int8_t vy)
    {
        _bullets[id] = BulletState{id, x, y, vx, vy};
    }

    void upsertMonster(int id, uint8_t x, uint8_t y, uint8_t hp, uint8_t type)
    {
        _monsters[id] = MonsterState{id, x, y, hp, type};
    }

    void clear()
    {
        _players.clear();
        _bullets.clear();
        _monsters.clear();
    }

    void clearPlayers()
    {
        _players.clear();
    }

    std::vector<PlayerState> listPlayers() const
    {
        std::vector<PlayerState> res;
        res.reserve(_players.size());
        for (const auto &kv : _players)
            res.push_back(kv.second);
        return res;
    }

    std::vector<BulletState> listBullets() const
    {
        std::vector<BulletState> res;
        res.reserve(_bullets.size());
        for (const auto &kv : _bullets)
            res.push_back(kv.second);
        return res;
    }

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
