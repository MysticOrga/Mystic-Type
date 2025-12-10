/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** GraphicClient
*/

#pragma once

#include "../Raylib/Raylib.hpp"
#include "../ecs/Core.hpp"
#include "../ecs/System.hpp"
#include "../../Network/Client/NetworkClient.hpp"
#include "../../Network/Client/GameState.hpp"
#include <unordered_map>
#include <string>

class GraphicClient {
public:
    GraphicClient(const std::string& ip, int port);
    ~GraphicClient() = default;

    int run();

private:
    bool init();
    void gameLoop();
    void processNetworkEvents();
    void updateEntities();
    void render();

    Entity createPlayerEntity(float x, float y);
    Entity createBulletEntity(float x, float y);

    void syncEntities(const std::vector<PlayerState> &players);
    void syncBullets(const std::vector<BulletState> &bullets);

    Raylib::Window _window;
    NetworkClient _net;
    GameState _state;
    ECS _ecs;

    // Systèmes
    SpriteRenderSystem _spriteRenderSystem;
    RectangleRenderSystem _rectangleRenderSystem;
    InputSystem _inputSystem;
    MovementSystem _movementSystem;

    std::unordered_map<int, Entity> _entities;
    std::unordered_map<int, Entity> _bulletEntities;
};