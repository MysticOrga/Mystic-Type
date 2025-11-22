/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** Components
*/

#ifndef COMPONENTS_HPP_
#define COMPONENTS_HPP_
#pragma once

class Position {
public:
  float x;
  float y;

  Position(float x = 0, float y = 0) : x(x), y(y) {}
};

class Velocity {
public:
  float vx;
  float vy;

  Velocity(float vx = 0, float vy = 0) : vx(vx), vy(vy) {}
};

class CircleComponent {
public:
    float radius;
    Color color;

    CircleComponent(float r = 20, Color c = RED)
        : radius(r), color(c) {}
};


#endif /* !COMPONENTS_HPP_ */
