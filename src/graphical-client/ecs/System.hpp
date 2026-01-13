/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** System
*/

#ifndef SYSTEM_HPP_
#define SYSTEM_HPP_
#pragma once
#include "../../Network/Client/NetworkClient.hpp"
#include "../Raylib/Raylib.hpp"
#include "Components.hpp"
#include "Core.hpp"
#include <algorithm>
#include <unordered_map>

enum KeyEvent
{
    RIGHT,
    LEFT,
    UP,
    DOWN,
    SHOOT
};

class InputSystem
{

  public:
    InputSystem()
    {
        _keyMap[RIGHT] = KEY_RIGHT;
        _keyMap[LEFT] = KEY_LEFT;
        _keyMap[UP] = KEY_UP;
        _keyMap[DOWN] = KEY_DOWN;
        _keyMap[SHOOT] = KEY_A;
    };

    inline void setKey(KeyEvent event, KeyboardKey key)
    {
        _keyMap[event] = key;
    };

    // [MODIFICATION] On prend maintenant "Velocity &vel" en paramètre pour la modifier directement
    void update(NetworkClient &net, const Position &pos, Velocity &vel)
    {
        bool moved = false;
        NetworkClient::MoveCmd cmd = NetworkClient::MoveCmd::Up;

        // Vitesse logique
        const int8_t speed = 5;

        // [MODIFICATION] Réinitialisation immédiate de la vélocité (arrêt net quand on relâche)
        vel.vx = 0;
        vel.vy = 0;

        // Calcul des inputs
        if (Raylib::Input::isKeyDown(_keyMap[RIGHT]))
        {
            cmd = NetworkClient::MoveCmd::Right;
            vel.vx = speed; // Application directe locale
            moved = true;
        }
        if (Raylib::Input::isKeyDown(_keyMap[LEFT]))
        {
            cmd = NetworkClient::MoveCmd::Left;
            vel.vx = -speed; // Application directe locale
            moved = true;
        }
        if (Raylib::Input::isKeyDown(_keyMap[UP]))
        {
            cmd = NetworkClient::MoveCmd::Up;
            vel.vy = -speed; // Application directe locale
            moved = true;
        }
        if (Raylib::Input::isKeyDown(_keyMap[DOWN]))
        {
            cmd = NetworkClient::MoveCmd::Down;
            vel.vy = speed; // Application directe locale
            moved = true;
        }

        // Envoi au serveur (pour qu'il valide et mette à jour les autres)
        if (moved)
        {
            _lastDir = cmd;
            // On envoie la vélocité que l'on vient de calculer
            net.sendInput(static_cast<uint8_t>(pos.x), static_cast<uint8_t>(pos.y), (int8_t)vel.vx, (int8_t)vel.vy,
                          cmd);
        }

        // Tir (Shooting)
        if (Raylib::Input::isKeyPressed(_keyMap[SHOOT]))
        {
            // On tire toujours vers l'axe X (à droite) avec une vitesse fixe
            const int8_t bvx = 2;
            const int8_t bvy = 0;
            // Décale légèrement le point de départ vers l'avant du sprite pour un spawn visuel cohérent
            float startX = pos.x + 4.0f;
            float startY = pos.y + 1.0f;
            net.sendShoot(static_cast<uint8_t>(std::clamp(startX, 0.0f, 255.0f)),
                          static_cast<uint8_t>(std::clamp(startY, 0.0f, 255.0f)), bvx, bvy);
        }
    }

  private:
    NetworkClient::MoveCmd _lastDir = NetworkClient::MoveCmd::Right;
    std::unordered_map<uint8_t, KeyboardKey> _keyMap;
};

// ... (Le reste des systèmes : MovementSystem, CircleRenderSystem, etc. reste inchangé) ...
class MovementSystem
{
  public:
    // Intègre les vitesses en suivant le pas de tick serveur (32 ms) pour éviter les accélérations/ralentissements
    void update(ECS &ecs, Entity e, float dt)
    {
        auto &pos = ecs.getComponent<Position>(e);
        auto &vel = ecs.getComponent<Velocity>(e);

        float &acc = _accumulator[e];
        acc += dt;
        constexpr float tickDuration = 0.032f; // 32 ms
        while (acc >= tickDuration)
        {
            pos.x += vel.vx;
            pos.y += vel.vy;
            acc -= tickDuration;
        }
    }

  private:
    std::unordered_map<Entity, float> _accumulator;
};

class CircleRenderSystem
{
  public:
    void update(ECS &ecs, Entity e)
    {
        auto &pos = ecs.getComponent<Position>(e);
        auto &circle = ecs.getComponent<CircleComponent>(e);
        Raylib::Draw::circle((int)(pos.x * 5), (int)(pos.y * 5), circle.radius, circle.color);
    }
};

class RectangleRenderSystem
{
  public:
    void setGameAreaOffset(float offsetX, float offsetY, float areaSize) 
    { 
        _offsetX = offsetX;
        _offsetY = offsetY;
        _areaSize = areaSize;
    }

    void update(ECS &ecs, Entity e)
    {
        auto &pos = ecs.getComponent<Position>(e);
        auto &rect = ecs.getComponent<RectangleComponent>(e);
        
        // Convert game coordinates (0-255) to screen coordinates
        float screenX = _offsetX + (pos.x / 255.0f) * _areaSize;
        float screenY = _offsetY + (pos.y / 255.0f) * _areaSize;
        
        Raylib::Draw::rectangle((int)screenX, (int)screenY, rect.width, rect.height, rect.color);
    }
  private:
    float _offsetX = 0.0f;
    float _offsetY = 0.0f;
    float _areaSize = 1280.0f;
};

class SpriteRenderSystem
{
  public:
    void setScale(float sx, float sy) { _scaleX = sx; _scaleY = sy; }
    
    void setGameAreaOffset(float offsetX, float offsetY, float areaSize)
    {
        _offsetX = offsetX;
        _offsetY = offsetY;
        _areaSize = areaSize;
    }

    void update(ECS &ecs, Entity e, float dt)
    {
        auto &pos = ecs.getComponent<Position>(e);
        auto &sprite = ecs.getComponent<Sprite>(e);

        if (!sprite.sprite) return;
        
        // Convert game coordinates (0-255) to screen coordinates
        float screenX = _offsetX + (pos.x / 255.0f) * _areaSize;
        float screenY = _offsetY + (pos.y / 255.0f) * _areaSize;
        
        sprite.sprite->setPosition({screenX, screenY});
        sprite.sprite->update(dt);
        sprite.sprite->draw();
    }

  private:
    float _scaleX = 1.0f;
    float _scaleY = 1.0f;
    float _offsetX = 0.0f;
    float _offsetY = 0.0f;
    float _areaSize = 1280.0f;
};

#endif /* !SYSTEM_HPP_ */
