/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** Components
*/

#ifndef COMPONENTS_HPP_
#define COMPONENTS_HPP_
#pragma once

#include "../Graphic/Graphic.hpp"
#include <raylib.h>
#include <memory>
#include <string>

/**
 * @class Position
 * @brief Represents the position of an entity in 2D space.
 *
 * This component stores the x and y coordinates of an entity.
 * Coordinates are typically in game space (0-255 range).
 */
class Position
{
  public:
    float x; ///< X coordinate
    float y; ///< Y coordinate

    /**
     * @brief Constructor for Position component.
     * @param x X coordinate (default 0.0)
     * @param y Y coordinate (default 0.0)
     */
    Position(float x = 0, float y = 0) : x(x), y(y) {}
};

/**
 * @class Sprite
 * @brief Component for rendering animated sprites.
 *
 * Contains a shared pointer to an AnimatedSprite object.
 */
class Sprite
{
  public:
    std::shared_ptr<Rtype::Graphic::AnimatedSprite> sprite; ///< Animated sprite pointer

    /**
     * @brief Default constructor.
     */
    Sprite() = default;

    /**
     * @brief Constructor with sprite initialization.
     * @param s Shared pointer to an AnimatedSprite
     */
    explicit Sprite(std::shared_ptr<Rtype::Graphic::AnimatedSprite> s) : sprite(std::move(s)) {}
};

/**
 * @class Velocity
 * @brief Represents the velocity of an entity.
 *
 * Stores velocity components in x and y directions.
 */
class Velocity
{
  public:
    float vx; ///< Velocity in X direction
    float vy; ///< Velocity in Y direction

    /**
     * @brief Constructor for Velocity component.
     * @param vx X velocity component (default 0.0)
     * @param vy Y velocity component (default 0.0)
     */
    Velocity(float vx = 0, float vy = 0) : vx(vx), vy(vy) {}
};

/**
 * @class CircleComponent
 * @brief Component for rendering circles.
 *
 * Used for debugging and simple circular entities.
 */
class CircleComponent
{
  public:
    float radius; ///< Radius of the circle
    Color color;  ///< Color of the circle

    /**
     * @brief Constructor for CircleComponent.
     * @param r Circle radius (default 20.0)
     * @param c Circle color (default RED)
     */
    CircleComponent(float r = 20, Color c = RED) : radius(r), color(c) {}
};

/**
 * @class RectangleComponent
 * @brief Component for rendering rectangles.
 *
 * Primarily used for projectile entities.
 */
class RectangleComponent
{
  public:
    int width;  ///< Width of the rectangle
    int height; ///< Height of the rectangle
    Color color; ///< Color of the rectangle

    /**
     * @brief Constructor for RectangleComponent.
     * @param w Width of the rectangle (default 10)
     * @param h Height of the rectangle (default 10)
     * @param c Color of the rectangle (default BLACK)
     */
    RectangleComponent(int w = 10, int h = 10, Color c = BLACK) 
        : width(w), height(h), color(c) {}
};

/**
 * @struct UIButton
 * @brief Data structure for UI button representation.
 *
 * Contains position, label, action identifier, and visual properties.
 */
struct UIButton {
    Rectangle bounds;            ///< Button bounds (position and size)
    std::string label;           ///< Button label text
    std::string action;          ///< Action identifier
    Color bg{40, 40, 60, 255};   ///< Background color
    Color hoverBg{70, 70, 120, 255}; ///< Hover background color
    Color text{230, 230, 230, 255}; ///< Text color
    bool hovered;                ///< Whether the button is currently hovered
};

/**
 * @struct UIText
 * @brief Data structure for UI text rendering.
 *
 * Contains position, text content, size, and color.
 */
struct UIText {
    Vector2 pos;                 ///< Text position
    std::string text;            ///< Text content
    int fontSize;                ///< Font size (default 22)
    Color color{230, 230, 230, 255}; ///< Text color
};

#endif /* !COMPONENTS_HPP_ */
