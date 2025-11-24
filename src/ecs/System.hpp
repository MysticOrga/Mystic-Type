/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** System
*/

#ifndef SYSTEM_HPP_
#define SYSTEM_HPP_
#pragma once
#include "Components.hpp"
#include "Core.hpp"
#include "raylib-cpp.hpp"

class InputSystem
{
  public:
    void update(ECS &ecs, Entity e)
    {
        auto &vel = ecs.getComponent<Velocity>(e);

        vel.vx = 0;
        vel.vy = 0;

        if (::IsKeyDown(KEY_RIGHT))
            vel.vx = 2;
        if (::IsKeyDown(KEY_LEFT))
            vel.vx = -2;
        if (::IsKeyDown(KEY_UP))
            vel.vy = -2;
        if (::IsKeyDown(KEY_DOWN))
            vel.vy = 2;
    }
};

class MovementSystem
{
  public:
    void update(ECS &ecs, Entity e)
    {
        auto &pos = ecs.getComponent<Position>(e);
        auto &vel = ecs.getComponent<Velocity>(e);

        pos.x += vel.vx;
        pos.y += vel.vy;
    }
};

class CircleRenderSystem
{
  public:
    void update(ECS &ecs, Entity e)
    {
        auto &pos = ecs.getComponent<Position>(e);
        auto &circle = ecs.getComponent<CircleComponent>(e);

        ::DrawCircle(pos.x, pos.y, circle.radius, circle.color);
    }
};

class RenderSystem
{
  public:
    void update(ECS &ecs, Entity e, ::Color color = RED)
    {
        auto &pos = ecs.getComponent<Position>(e);
        ::DrawRectangle(pos.x, pos.y, 60, 60, color);
    }
};

class SpriteRenderSystem
{
  public:
    void update(ECS &ecs, Entity e)
    {
        auto &pos = ecs.getComponent<Position>(e);
        auto &sprite = ecs.getComponent<Sprite>(e);

        if (!sprite.sprite)
            return;

        sprite.sprite->setPosition({pos.x, pos.y});
        sprite.sprite->update();
        sprite.sprite->draw();
    }
};

#endif /* !SYSTEM_HPP_ */
