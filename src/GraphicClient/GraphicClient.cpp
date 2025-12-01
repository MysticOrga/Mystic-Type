/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** GraphicClient
*/

#include "GraphicClient.hpp"
#include "../ecs/Components.hpp"
#include <iostream>
#include <memory>

GraphicClient::GraphicClient(const std::string& ip, int port)
    : _window(1920, 1080, "Mystic-Type"), _net(ip, port)
{
    ::SetTargetFPS(60);
}

bool GraphicClient::init()
{
    if (!::IsWindowReady()) {
        std::cerr << "Failed to initialize window." << std::endl;
        return false;
    }
    if (!_net.connectToServer()) {
        std::cerr << "[CLIENT] Failed to connect\n";
        return false;
    }
    if (!_net.performHandshake()) {
        std::cerr << "[CLIENT] Handshake failed\n";
        return false;
    }
    std::cout << "[CLIENT] Assigned ID " << _net.getPlayerId() << "\n";
    _net.sendHelloUdp(0, 0);

    _net.pollPackets();
    for (const auto &p : _net.getLastPlayerList())
        _state.upsertPlayer(p.id, p.x, p.y);
    syncEntities(_state.listPlayers());
    _net.clearEvents();
    return true;
}

Entity GraphicClient::createPlayerEntity(float x, float y)
{
    Entity ent = _ecs.createEntity();
    _ecs.addComponent(ent, Position{x, y});
    _ecs.addComponent(ent, Velocity{0, 0});

    auto sprite = std::make_shared<Rtype::Graphic::AnimatedSprite>(
        "../sprites/r-typesheet42.gif",
        raylib::Vector2{33, 17},
        raylib::Vector2{0, 0},
        4,
        0.15f,
        raylib::Vector2{x, y}
    );
    _ecs.addComponent(ent, Sprite{sprite});
    return ent;
}

void GraphicClient::syncEntities(const std::vector<PlayerState> &players)
{
    for (const auto &p : players) {
        if (_entities.size() >= 4 && _entities.find(p.id) == _entities.end())
            continue;

        float clientX = static_cast<float>(p.x) * (static_cast<float>(_window.GetWidth()) / 255.0f);
        float clientY = static_cast<float>(p.y) * (static_cast<float>(_window.GetHeight()) / 255.0f);

        auto it = _entities.find(p.id);
        if (it == _entities.end()) {
            Entity ent = createPlayerEntity(clientX, clientY);
            _entities[p.id] = ent;
            std::cout << "[CLIENT] Spawned player " << p.id << " at (" << static_cast<int>(clientX)
                      << "," << static_cast<int>(clientY) << ") from server pos ("
                      << static_cast<int>(p.x) << "," << static_cast<int>(p.y) << ")\n";
        } else {
            auto &pos = _ecs.getComponent<Position>(it->second);
            pos.x = clientX;
            pos.y = clientY;
        }
    }
}

void GraphicClient::processNetworkEvents()
{
    _net.pollPackets();
    for (const auto &ev : _net.getEvents()) {
        if (ev == "PLAYER_LIST" || ev == "NEW_PLAYER") {
            for (const auto &p : _net.getLastPlayerList())
                _state.upsertPlayer(p.id, p.x, p.y);
        } else if (ev == "SNAPSHOT") {
            for (const auto &p : _net.getLastSnapshot())
                _state.upsertPlayer(p.id, p.x, p.y);
        }
    }
    _net.clearEvents();
}

void GraphicClient::updateEntities()
{
    syncEntities(_state.listPlayers());
    _inputSystem.update(_net);
}

void GraphicClient::render()
{
    _window.BeginDrawing();
    _window.ClearBackground(RAYWHITE);
    for (const auto &kv : _entities) {
        _spriteRenderSystem.update(_ecs, kv.second);
    }
    _window.EndDrawing();
}

void GraphicClient::gameLoop()
{
    while (!_window.ShouldClose()) {
        processNetworkEvents();
        updateEntities();
        render();
    }
}

int GraphicClient::run()
{
    if (!init()) {
        return 1;
    }
    gameLoop();
    return 0;
}
