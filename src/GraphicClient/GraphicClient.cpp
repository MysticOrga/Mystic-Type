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
    _window.setTargetFPS(60);
}

bool GraphicClient::init()
{
    // On assume que le constructeur de _window a réussi (InitWindow ne renvoie pas de bool, mais IsWindowReady peut être checké)
    // Notre wrapper Window gère l'init.
    
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

    // Note: Vector2{x, y} est maintenant la struct C native via raylib.h
    auto sprite = std::make_shared<Rtype::Graphic::AnimatedSprite>(
        "../sprites/r-typesheet42.gif",
        Vector2{33, 17},
        Vector2{0, 0},
        4,
        0.15f,
        Vector2{x, y}
    );
    _ecs.addComponent(ent, Sprite{sprite});
    return ent;
}

void GraphicClient::syncEntities(const std::vector<PlayerState> &players)
{
    for (const auto &p : players) {
        if (_entities.size() >= 4 && _entities.find(p.id) == _entities.end())
            continue;

        // Appel modifié: _window.getWidth() au lieu de GetWidth()
        float clientX = static_cast<float>(p.x) * (static_cast<float>(_window.getWidth()) / 255.0f);
        float clientY = static_cast<float>(p.y) * (static_cast<float>(_window.getHeight()) / 255.0f);

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
    int myId = _net.getPlayerId();

    if (_entities.find(myId) != _entities.end()) {
        Entity myEntity = _entities[myId];
        const auto &myPos = _ecs.getComponent<Position>(myEntity);
        _inputSystem.update(_net, myPos);
    }
}

void GraphicClient::render()
{
    _window.beginDrawing();
    _window.clearBackground(RAYWHITE); // RAYWHITE vient de raylib.h
    
    float dt = _window.getFrameTime();
    
    for (const auto &kv : _entities) {
        _spriteRenderSystem.update(_ecs, kv.second, dt);
    }
    _window.endDrawing();
}

void GraphicClient::gameLoop()
{
    while (!_window.shouldClose()) {
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