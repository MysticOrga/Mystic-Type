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
#include "../Client/NetworkClient.hpp"
#include "../Client/GameState.hpp"
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
    void syncEntities(const std::vector<PlayerState> &players);

    // Changement ici: Raylib::Window au lieu de raylib::Window
    Raylib::Window _window;
    NetworkClient _net;
    GameState _state;
    ECS _ecs;
    SpriteRenderSystem _spriteRenderSystem;
    InputSystem _inputSystem;
    std::unordered_map<int, Entity> _entities;
};