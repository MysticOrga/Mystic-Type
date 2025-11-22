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

class InputSystem {
public:
  void update(ECS &ecs, Entity e, Raylib::Core &ray) {
    auto &vel = ecs.getComponent<Velocity>(e);

    vel.vx = 0;
    vel.vy = 0;

    if (ray.isKeyDown(KEY_RIGHT))
      vel.vx = 2;
    if (ray.isKeyDown(KEY_LEFT))
      vel.vx = -2;
    if (ray.isKeyDown(KEY_UP))
      vel.vy = -2;
    if (ray.isKeyDown(KEY_DOWN))
      vel.vy = 2;
  }
};

class MovementSystem {
public:
  void update(ECS &ecs, Entity e) {
    auto &pos = ecs.getComponent<Position>(e);
    auto &vel = ecs.getComponent<Velocity>(e);

    pos.x += vel.vx;
    pos.y += vel.vy;
  }
};

class CircleRenderSystem {
public:
    void update(ECS& ecs, Entity e, Raylib::Core& ray) {
        auto& pos = ecs.getComponent<Position>(e);
        auto& circle = ecs.getComponent<CircleComponent>(e);

        ray.drawCircle(pos.x, pos.y, circle.radius, circle.color);
    }
};

class RenderSystem {
public:
  void update(ECS &ecs, Entity e, Raylib::Core &ray, Raylib::Color color = RED) {
    auto &pos = ecs.getComponent<Position>(e);
    ray.drawRectangle(pos.x, pos.y, 60, 60, color);
  }
};

#endif /* !SYSTEM_HPP_ */
