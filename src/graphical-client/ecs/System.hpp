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
#include "../../Network/Client/NetworkClient.hpp"


class InputSystem
{
  public:
    void update(NetworkClient &net, const Position &pos)
    {
        bool moved = false;
        NetworkClient::MoveCmd cmd = NetworkClient::MoveCmd::Up;
        int8_t velX = 0;
        int8_t velY = 0;

        const int8_t speed = 4; 

        if (Raylib::Input::isKeyDown(KEY_D)) { 
            cmd = NetworkClient::MoveCmd::Right; 
            velX = speed;
            moved = true; 
        }
        if (Raylib::Input::isKeyDown(KEY_A)) { 
            cmd = NetworkClient::MoveCmd::Left; 
            velX = -speed;
            moved = true; 
        }
        if (Raylib::Input::isKeyDown(KEY_W)) { 
            cmd = NetworkClient::MoveCmd::Up; 
            velY = -speed; 
            moved = true; 
        }
        if (Raylib::Input::isKeyDown(KEY_S)) { 
            cmd = NetworkClient::MoveCmd::Down; 
            velY = speed; 
            moved = true; 
        }

        if (moved) {
            _lastDir = cmd; // Update direction
            net.sendInput(static_cast<uint8_t>(pos.x), static_cast<uint8_t>(pos.y), velX, velY, cmd);
        }

        // Shooting Mechanic
        if (Raylib::Input::isKeyPressed(KEY_SPACE)) {
            int8_t bvx = 0;
            int8_t bvy = 0;

            switch (_lastDir) {
                case NetworkClient::MoveCmd::Right: bvx = 2; break;
                case NetworkClient::MoveCmd::Left:  bvx = -2; break;
                case NetworkClient::MoveCmd::Down:  bvy = 2; break;
                case NetworkClient::MoveCmd::Up:    bvy = -2; break;
            }
            
            net.sendShoot(static_cast<uint8_t>(pos.x), static_cast<uint8_t>(pos.y), bvx, bvy);
        }
    }

  private:
    NetworkClient::MoveCmd _lastDir = NetworkClient::MoveCmd::Right; // Track last direction
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

class RectangleRenderSystem
{
  public:
    void update(ECS &ecs, Entity e)
    {
        auto &pos = ecs.getComponent<Position>(e);
        auto &rect = ecs.getComponent<RectangleComponent>(e);

        Raylib::Draw::rectangle((int)pos.x, (int)pos.y, rect.width, rect.height, rect.color);
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

        // sprite.sprite->setPosition({pos.x * _scaleX, pos.y * _scaleY});
        sprite.sprite->setPosition({pos.x, pos.y});
        sprite.sprite->update(dt);
        sprite.sprite->draw();
    }

  private:
    float _scaleX = 1.0f;
    float _scaleY = 1.0f;
};

#endif /* !SYSTEM_HPP_ */