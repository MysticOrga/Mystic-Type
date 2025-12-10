/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** GraphicClient
*/

#include "graphical-client/GraphicClient/GraphicClient.hpp"
#include "graphical-client/ecs/Components.hpp"
#include <cmath> // Pour std::abs si besoin, mais ici calcul simple
#include <iostream>
#include <memory>
#include <unordered_set>

GraphicClient::GraphicClient(const std::string &ip, int port) : _window(1920, 1080, "Mystic-Type"), _net(ip, port)
{
    _window.setTargetFPS(60);
}

bool GraphicClient::init()
{
    if (!_net.connectToServer())
    {
        std::cerr << "[CLIENT] Failed to connect\n";
        return false;
    }
    if (!_net.performHandshake())
    {
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
        "../../../sprites/r-typesheet42.gif", Vector2{33, 17}, Vector2{0, 0}, 4, 0.15f, Vector2{x, y});
    _ecs.addComponent(ent, Sprite{sprite});
    return ent;
}

Entity GraphicClient::createBulletEntity(float x, float y)
{
    Entity ent = _ecs.createEntity();
    _ecs.addComponent(ent, Position{x, y});
    _ecs.addComponent(ent, Velocity{0, 0});
    _ecs.addComponent(ent, RectangleComponent{6, 6, BLACK});
    return ent;
}

void GraphicClient::syncEntities(const std::vector<PlayerState> &players)
{
    for (const auto &p : players)
    {
        if (_entities.size() >= 4 && _entities.find(p.id) == _entities.end())
            continue;

        float clientX = static_cast<float>(p.x);
        float clientY = static_cast<float>(p.y);

        auto it = _entities.find(p.id);
        if (it == _entities.end())
        {
            Entity ent = createPlayerEntity(clientX, clientY);
            _entities[p.id] = ent;
            std::cout << "[CLIENT] Spawned player " << p.id << "\n";
        }
        else
        {
            auto &pos = _ecs.getComponent<Position>(it->second);
            auto &vel = _ecs.getComponent<Velocity>(it->second);

            float smoothingFactor = 0.1f;

            vel.vx = (clientX - pos.x) * smoothingFactor;
            vel.vy = (clientY - pos.y) * smoothingFactor;
        }
    }
}

void GraphicClient::syncBullets(const std::vector<BulletState> &bullets)
{
    std::unordered_set<int> liveIds;

    for (const auto &b : bullets)
    {
        liveIds.insert(b.id);
        float clientX = static_cast<float>(b.x);
        float clientY = static_cast<float>(b.y);

        auto it = _bulletEntities.find(b.id);
        if (it == _bulletEntities.end())
        {
            Entity ent = createBulletEntity(clientX, clientY);
            _bulletEntities[b.id] = ent;
        }
        else
        {
            auto &pos = _ecs.getComponent<Position>(it->second);
            pos.x = clientX;
            pos.y = clientY;
        }
    }

    for (auto &kv : _bulletEntities)
    {
        if (liveIds.find(kv.first) == liveIds.end())
        {
            auto &pos = _ecs.getComponent<Position>(kv.second);
            pos.x = -1000.0f;
            pos.y = -1000.0f;
        }
    }
}

void GraphicClient::processNetworkEvents()
{
    _net.pollPackets();
    for (const auto &ev : _net.getEvents())
    {
        if (ev == "PLAYER_LIST" || ev == "NEW_PLAYER")
        {
            for (const auto &p : _net.getLastPlayerList())
                _state.upsertPlayer(p.id, p.x, p.y);
        }
        else if (ev == "SNAPSHOT")
        {
            for (const auto &p : _net.getLastSnapshot())
                _state.upsertPlayer(p.id, p.x, p.y);
        }
    }
    _net.clearEvents();
}

void GraphicClient::updateEntities()
{
    syncEntities(_state.listPlayers());
    syncBullets(_net.getLastSnapshotBullets());

    for (const auto &kv : _entities)
    {
        _movementSystem.update(_ecs, kv.second);
    }
    for (const auto &kv : _bulletEntities)
    {
        _movementSystem.update(_ecs, kv.second);
    }
    int myId = _net.getPlayerId();
    if (_entities.find(myId) != _entities.end())
    {
        Entity myEntity = _entities[myId];
        const auto &myPos = _ecs.getComponent<Position>(myEntity);
        _inputSystem.update(_net, myPos);
    }
}

void GraphicClient::render()
{
    _window.beginDrawing();
    _window.clearBackground(RAYWHITE);
    float dt = _window.getFrameTime();

    _spriteRenderSystem.setScale(2.0f, 2.0f);

    for (const auto &kv : _entities)
    {
        _spriteRenderSystem.update(_ecs, kv.second, dt);
    }
    for (const auto &kv : _bulletEntities)
    {
        _rectangleRenderSystem.update(_ecs, kv.second);
    }

    _window.endDrawing();
}

void GraphicClient::gameLoop()
{
    while (!_window.shouldClose())
    {
        processNetworkEvents();
        updateEntities();
        render();
    }
}

int GraphicClient::run()
{
    if (!init())
    {
        return 1;
    }
    gameLoop();
    return 0;
}