/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** Client entry point
*/

#include "raylib-cpp.hpp"
#include "./Graphic/Graphic.hpp"
#include "./ecs/Core.hpp"
#include "./ecs/Components.hpp"
#include "./ecs/System.hpp"
#include "Client/NetworkClient.hpp"
#include "Client/GameState.hpp"
#include <chrono>
#include <iostream>
#include <unordered_map>
#include <memory>

static Entity createPlayerEntity(ECS &ecs, float x, float y)
{
    Entity ent = ecs.createEntity();
    ecs.addComponent(ent, Position{x, y});
    ecs.addComponent(ent, Velocity{0, 0});

    auto sprite = std::make_shared<Rtype::Graphic::AnimatedSprite>(
        "../sprites/r-typesheet42.gif",
        raylib::Vector2{33, 17},
        raylib::Vector2{0, 0},
        4,
        0.15f,
        raylib::Vector2{x, y}
    );
    ecs.addComponent(ent, Sprite{sprite});
    return ent;
}

static void syncEntities(ECS &ecs, std::unordered_map<int, Entity> &entities, const std::vector<PlayerState> &players, int windowWidth, int windowHeight)
{
    for (const auto &p : players) {
        if (entities.size() >= 4 && entities.find(p.id) == entities.end())
            continue;

        float clientX = static_cast<float>(p.x) * (static_cast<float>(windowWidth) / 255.0f);
        float clientY = static_cast<float>(p.y) * (static_cast<float>(windowHeight) / 255.0f);

        auto it = entities.find(p.id);
        if (it == entities.end()) {
            Entity ent = createPlayerEntity(ecs, clientX, clientY);
            entities[p.id] = ent;
            std::cout << "[CLIENT] Spawned player " << p.id << " at (" << static_cast<int>(clientX)
                      << "," << static_cast<int>(clientY) << ") from server pos ("
                      << static_cast<int>(p.x) << "," << static_cast<int>(p.y) << ")\n";
        } else {
            auto &pos = ecs.getComponent<Position>(it->second);
            pos.x = clientX;
            pos.y = clientY;
            // Optional: reduce log spam
            // std::cout << "[CLIENT] Updated player " << p.id << " to (" << static_cast<int>(clientX)
            //           << "," << static_cast<int>(clientY) << ")\n";
        }
    }
}

int main()
{
    NetworkClient net("127.0.0.1", 4242);
    GameState state;

    if (!net.connectToServer()) {
        std::cerr << "[CLIENT] Failed to connect\n";
        return 1;
    }
    if (!net.performHandshake()) {
        std::cerr << "[CLIENT] Handshake failed\n";
        return 1;
    }
    std::cout << "[CLIENT] Assigned ID " << net.getPlayerId() << "\n";
    net.sendHelloUdp(0, 0);

    raylib::Window window(800, 600, "Mystic-Type");
    ::SetTargetFPS(60);
    if (!::IsWindowReady()) {
        std::cerr << "Failed to initialize window." << std::endl;
        return 1;
    }

    ECS ecs;
    SpriteRenderSystem spriteRenderSystem;
    InputSystem inputSystem;
    std::unordered_map<int, Entity> entities;

    net.pollPackets();
    for (const auto &p : net.getLastPlayerList())
        state.upsertPlayer(p.id, p.x, p.y);
    syncEntities(ecs, entities, state.listPlayers(), window.GetWidth(), window.GetHeight());
    net.clearEvents();

    while (!window.ShouldClose()) {
        net.pollPackets();
        for (const auto &ev : net.getEvents()) {
            if (ev == "PLAYER_LIST" || ev == "NEW_PLAYER") {
                for (const auto &p : net.getLastPlayerList())
                    state.upsertPlayer(p.id, p.x, p.y);
            } else if (ev == "SNAPSHOT") {
                for (const auto &p : net.getLastSnapshot())
                    state.upsertPlayer(p.id, p.x, p.y);
            }
        }
        net.clearEvents();
        syncEntities(ecs, entities, state.listPlayers(), window.GetWidth(), window.GetHeight());

        inputSystem.update(net);

        window.BeginDrawing();
        window.ClearBackground(RAYWHITE);
        for (const auto &kv : entities) {
            if (kv.first == net.getPlayerId()) {
                spriteRenderSystem.update(ecs, kv.second);
            } else {
                spriteRenderSystem.update(ecs, kv.second, {255, 255, 255, 150});
            }
        }
        window.EndDrawing();
    }

    return 0;
}
