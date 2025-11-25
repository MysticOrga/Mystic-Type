/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** Game state for client
*/

#pragma once

#include <unordered_map>
#include <vector>

struct PlayerState {
    int id = 0;
    uint8_t x = 0;
    uint8_t y = 0;
};

class GameState {
public:
    void upsertPlayer(int id, uint8_t x, uint8_t y)
    {
        _players[id] = PlayerState{id, x, y};
    }

    void clear()
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

private:
    std::unordered_map<int, PlayerState> _players;
};
