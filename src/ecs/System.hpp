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
#include "../Client/NetworkClient.hpp"

class InputSystem
{
  public:
    void update(NetworkClient &net, const Position &pos)
    {
        bool moved = false;
        NetworkClient::MoveCmd cmd = NetworkClient::MoveCmd::Up;
        int8_t velX = 0;
        int8_t velY = 0;

        if (::IsKeyDown(KEY_D)) { 
            cmd = NetworkClient::MoveCmd::Right; 
            velX = 1; 
            moved = true; 
        }
        if (::IsKeyDown(KEY_A)) { 
            cmd = NetworkClient::MoveCmd::Left; 
            velX = -1; 
            moved = true; 
        }
        if (::IsKeyDown(KEY_W)) { 
            cmd = NetworkClient::MoveCmd::Up; 
            velY = -1; 
            moved = true; 
        }
        if (::IsKeyDown(KEY_S)) { 
            cmd = NetworkClient::MoveCmd::Down; 
            velY = 1; 
            moved = true; 
        }

        if (moved) {
            net.sendInput(static_cast<uint8_t>(pos.x), static_cast<uint8_t>(pos.y), velX, velY, cmd);
        }
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

    void update(ECS &ecs, Entity e, ::Color color)
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