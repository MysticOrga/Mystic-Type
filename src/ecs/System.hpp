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
#include "../Raylib/Raylib.hpp"
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

        if (Raylib::Input::isKeyDown(KEY_D)) { 
            cmd = NetworkClient::MoveCmd::Right; 
            velX = 1; 
            moved = true; 
        }
        if (Raylib::Input::isKeyDown(KEY_A)) { 
            cmd = NetworkClient::MoveCmd::Left; 
            velX = -1; 
            moved = true; 
        }
        if (Raylib::Input::isKeyDown(KEY_W)) { 
            cmd = NetworkClient::MoveCmd::Up; 
            velY = -1; 
            moved = true; 
        }
        if (Raylib::Input::isKeyDown(KEY_S)) { 
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

        Raylib::Draw::circle((int)pos.x, (int)pos.y, circle.radius, circle.color);
    }
};

class RenderSystem
{
  public:
    void update(ECS &ecs, Entity e, Color color = RED)
    {
        auto &pos = ecs.getComponent<Position>(e);
        Raylib::Draw::rectangle((int)pos.x, (int)pos.y, 60, 60, color);
    }
};

class SpriteRenderSystem
{
  public:
    void setScale(float sx, float sy)
    {
        _scaleX = sx;
        _scaleY = sy;
    }

    void update(ECS &ecs, Entity e, float dt)
    {
        auto &pos = ecs.getComponent<Position>(e);
        auto &sprite = ecs.getComponent<Sprite>(e);

        if (!sprite.sprite)
            return;

        // Render using scaled coordinates so server-space (0-255) fills the window.
        sprite.sprite->setPosition({pos.x * _scaleX, pos.y * _scaleY});
        sprite.sprite->update(dt);
        sprite.sprite->draw();
    }

  private:
    float _scaleX = 1.0f;
    float _scaleY = 1.0f;
};

#endif /* !SYSTEM_HPP_ */
