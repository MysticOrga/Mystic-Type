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
        "./sprites/r-typesheet42.gif",
        raylib::Vector2{33, 17},
        raylib::Vector2{0, 0},
        5,
        0.15f,
        raylib::Vector2{x, y}
    );
    ecs.addComponent(ent, Sprite{sprite});
    return ent;
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
    std::unordered_map<int, Entity> entities;

    auto syncEntities = [&](const std::vector<PlayerState> &players) {
        for (const auto &p : players) {
            if (entities.size() >= 4 && entities.find(p.id) == entities.end())
                continue;
            auto it = entities.find(p.id);
            if (it == entities.end()) {
                Entity ent = createPlayerEntity(ecs, p.x, p.y);
                entities[p.id] = ent;
                std::cout << "[CLIENT] Spawned player " << p.id << " at (" << static_cast<int>(p.x)
                          << "," << static_cast<int>(p.y) << ")\n";
            } else {
                auto &pos = ecs.getComponent<Position>(it->second);
                pos.x = p.x;
                pos.y = p.y;
            }
        }
    };
    net.pollPackets();
    for (const auto &p : net.getLastPlayerList())
        state.upsertPlayer(p.id, p.x, p.y);
    syncEntities(state.listPlayers());
    net.clearEvents();

    uint8_t posX = 0;
    uint8_t posY = 0;
    auto lastInputSend = std::chrono::steady_clock::now();
    while (!window.ShouldClose()) {
        net.pollPackets();
        for (const auto &ev : net.getEvents()) {
            if (ev == "PLAYER_LIST" || ev == "NEW_PLAYER") {
                for (const auto &p : net.getLastPlayerList())
                    state.upsertPlayer(p.id, p.x, p.y);
            } else if (ev == "SNAPSHOT") {
                for (const auto &p : net.getLastSnapshot())
                    state.upsertPlayer(p.id, p.x, p.y);
                std::cout << "[CLIENT] Snapshot: ";
                for (const auto &p : state.listPlayers())
                    std::cout << p.id << "(" << static_cast<int>(p.x) << "," << static_cast<int>(p.y) << ") ";
                std::cout << "\n";
            }
        }
        net.clearEvents();
        syncEntities(state.listPlayers());
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastInputSend).count() >= 1000) {
            posX = static_cast<uint8_t>(posX + 1);
            net.sendInput(posX, posY);
            lastInputSend = now;
        }
        window.BeginDrawing();
        window.ClearBackground(RAYWHITE);
        for (const auto &kv : entities)
            spriteRenderSystem.update(ecs, kv.second);
        window.EndDrawing();
    }

    return 0;
}
