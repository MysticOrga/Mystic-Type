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
        if (Raylib::Input::isKeyDown(KEY_D)) { 
            cmd = NetworkClient::MoveCmd::Right; 
            vel.vx = speed; // Application directe locale
            moved = true; 
        }
        if (Raylib::Input::isKeyDown(KEY_A)) { 
            cmd = NetworkClient::MoveCmd::Left; 
            vel.vx = -speed; // Application directe locale
            moved = true; 
        }
        if (Raylib::Input::isKeyDown(KEY_W)) { 
            cmd = NetworkClient::MoveCmd::Up; 
            vel.vy = -speed; // Application directe locale
            moved = true; 
        }
        if (Raylib::Input::isKeyDown(KEY_S)) { 
            cmd = NetworkClient::MoveCmd::Down; 
            vel.vy = speed; // Application directe locale
            moved = true; 
        }

        // Envoi au serveur (pour qu'il valide et mette à jour les autres)
        if (moved) {
            _lastDir = cmd;
            // On envoie la vélocité que l'on vient de calculer
            net.sendInput(static_cast<uint8_t>(pos.x), static_cast<uint8_t>(pos.y), (int8_t)vel.vx, (int8_t)vel.vy, cmd);
        }

        // Tir (Shooting)
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
    NetworkClient::MoveCmd _lastDir = NetworkClient::MoveCmd::Right;
};

// ... (Le reste des systèmes : MovementSystem, CircleRenderSystem, etc. reste inchangé) ...
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
        Raylib::Draw::circle((int)(pos.x * 5), (int)(pos.y * 5), circle.radius, circle.color);
    }
};

class RectangleRenderSystem
{
  public:
    void update(ECS &ecs, Entity e)
    {
        auto &pos = ecs.getComponent<Position>(e);
        auto &rect = ecs.getComponent<RectangleComponent>(e);
        Raylib::Draw::rectangle((int)(pos.x * 5), (int)(pos.y * 5), rect.width, rect.height, rect.color);
    }
};

class SpriteRenderSystem
{
  public:
    void setScale(float sx, float sy) { _scaleX = sx; _scaleY = sy; }

    void update(ECS &ecs, Entity e, float dt)
    {
        auto &pos = ecs.getComponent<Position>(e);
        auto &sprite = ecs.getComponent<Sprite>(e);

        if (!sprite.sprite) return;
        
        sprite.sprite->setPosition({pos.x * 5.0f, pos.y * 5.0f});
        sprite.sprite->update(dt);
        sprite.sprite->draw();
    }
  private:
    float _scaleX = 1.0f;
    float _scaleY = 1.0f;
};

#endif /* !SYSTEM_HPP_ */