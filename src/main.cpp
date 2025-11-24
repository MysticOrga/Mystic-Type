/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** main
*/

#include "raylib-cpp.hpp"
#include "./Graphic/Graphic.hpp"
#include "./ecs/Core.hpp"
#include "./ecs/Components.hpp"
#include "./ecs/System.hpp"
#include <iostream>
#include <memory>

int main()
{
    raylib::Window window(800, 600, "Raylib-cpp Ready");
    ::SetTargetFPS(60);
    if (!::IsWindowReady())
    {
        std::cerr << "Failed to initialize window." << std::endl;
        return 1;
    }

    ECS ecs;
    Entity player = ecs.createEntity();
    ecs.addComponent(player, Position{100, 100});
    ecs.addComponent(player, Velocity{0, 0});

    auto playerSprite = std::make_shared<Rtype::Graphic::AnimatedSprite>(
        "./sprites/r-typesheet42.gif",
        raylib::Vector2{33, 17},
        raylib::Vector2{0, 0},
        5,
        0.15f,
        raylib::Vector2{100, 100});
    ecs.addComponent(player, Sprite{playerSprite});

    InputSystem inputSystem;
    MovementSystem movementSystem;
    SpriteRenderSystem spriteRenderSystem;

    while (!window.ShouldClose())
    {
        inputSystem.update(ecs, player);
        movementSystem.update(ecs, player);

        window.BeginDrawing();
        window.ClearBackground(RAYWHITE);
        spriteRenderSystem.update(ecs, player);
        window.EndDrawing();
    }
    return 0;
}
