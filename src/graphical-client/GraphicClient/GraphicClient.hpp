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
#include <chrono>
#include <vector>

class GraphicClient {
public:
    GraphicClient(const std::string& ip, int port);
    ~GraphicClient() = default;

    int run();
    static constexpr float GAME_AREA_SIZE = 765.0f;
    static constexpr float GAME_AREA_OFFSET_X = 577.5f;
    static constexpr float GAME_AREA_OFFSET_Y = 157.5f;

private:
    bool init();
    void gameLoop();
    void processNetworkEvents();
    void updateEntities(float dt);
    void render(float dt);
    void drawGameBackground(float hoverAnimTimer);

    Entity createPlayerEntity(float x, float y);
    Entity createBulletEntity(float x, float y, float vx, float vy);
    Entity createMonsterEntity(float x, float y, uint8_t type);

    void syncEntities(const std::vector<PlayerState> &players);
    void syncBullets(const std::vector<BulletState> &bullets);
    void syncMonsters(const std::vector<MonsterState> &monsters);
    bool selectLobby();
    bool selectPseudo();

    Raylib::Window _window;
    NetworkClient _net;
    GameState _state;
    ECS _ecs;
    ECS _uiEcs;

    SpriteRenderSystem _spriteRenderSystem;
    RectangleRenderSystem _rectangleRenderSystem;
    InputSystem _inputSystem;
    MovementSystem _movementSystem;

    std::unordered_map<int, Entity> _entities;
    std::unordered_map<int, Entity> _bulletEntities;
    std::unordered_map<int, Entity> _monsterEntities;
    std::vector<std::string> _chatLog;
    std::string _chatInput;
    bool _chatActive = false;
    std::string _localPseudo;
    std::string _lastChatSent;
    std::unordered_map<int, int> _playerPingMs;
    std::chrono::steady_clock::time_point _lastKeepAlive{};
    std::chrono::steady_clock::time_point _lastHello{};
    bool _udpReady = false;
    float _gameAnimTimer = 0.0f;
    bool _forceExit = false;
};
