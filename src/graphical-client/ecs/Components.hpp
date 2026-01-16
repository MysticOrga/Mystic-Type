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
* @brief Represents the 2D position of an entity.
*
* Stores X and Y coordinates for game world positioning.
*/
class Position
{
  public:
    float x; /**< X coordinate */
    float y; /**< Y coordinate */

    /**
    * @brief Constructor for Position.
    * @param x X coordinate (default: 0).
    * @param y Y coordinate (default: 0).
    */
    Position(float x = 0, float y = 0) : x(x), y(y) {}
};

/**
* @class Sprite
* @brief Stores an animated sprite for rendering.
*
* Contains a shared pointer to an AnimatedSprite object.
*/
class Sprite
{
  public:
    std::shared_ptr<Rtype::Graphic::AnimatedSprite> sprite; /**< Animated sprite pointer */

    /**
    * @brief Default constructor.
    */
    Sprite() = default;
    
    /**
    * @brief Constructor with sprite initialization.
    * @param s Shared pointer to AnimatedSprite.
    */
    explicit Sprite(std::shared_ptr<Rtype::Graphic::AnimatedSprite> s) : sprite(std::move(s)) {}
};

/**
* @class Velocity
* @brief Represents the velocity of an entity.
*
* Stores X and Y velocity components for movement.
*/
class Velocity
{
  public:
    float vx; /**< X velocity component */
    float vy; /**< Y velocity component */

    /**
    * @brief Constructor for Velocity.
    * @param vx X velocity (default: 0).
    * @param vy Y velocity (default: 0).
    */
    Velocity(float vx = 0, float vy = 0) : vx(vx), vy(vy) {}
};

/**
* @class CircleComponent
* @brief Component for rendering circular entities.
*
* Stores visual properties for circle rendering.
*/
class CircleComponent
{
  public:
    float radius;  /**< Circle radius in pixels */
    Color color;   /**< Circle color */

    /**
    * @brief Constructor for CircleComponent.
    * @param r Radius (default: 20).
    * @param c Color (default: RED).
    */
    CircleComponent(float r = 20, Color c = RED) : radius(r), color(c) {}
};

/**
* @class RectangleComponent
* @brief Component for rendering rectangular entities.
*
* Stores visual properties for rectangle rendering.
*/
class RectangleComponent
{
  public:
    int width;   /**< Rectangle width */
    int height;  /**< Rectangle height */
    Color color; /**< Rectangle color */

    /**
    * @brief Constructor for RectangleComponent.
    * @param w Width (default: 10).
    * @param h Height (default: 10).
    * @param c Color (default: BLACK).
    */
    RectangleComponent(int w = 10, int h = 10, Color c = BLACK) 
        : width(w), height(h), color(c) {}
};

/**
* @struct UIButton
* @brief Component for UI button elements.
*
* Represents a clickable button in the user interface.
*/
struct UIButton {
    Rectangle bounds;             /**< Button boundaries */
    std::string label;            /**< Button label text */
    std::string action;           /**< Associated action identifier */
    Color bg{40, 40, 60, 255};           /**< Background color */
    Color hoverBg{70, 70, 120, 255};     /**< Hover background color */
    Color text{230, 230, 230, 255};      /**< Text color */
    bool hovered = false;         /**< Hover state */
};

/**
* @struct UIText
* @brief Component for UI text elements.
*
* Represents text displayed in the user interface.
*/
struct UIText {
    Vector2 pos;                      /**< Text position */
    std::string text;                 /**< Text content */
    int fontSize = 22;                /**< Font size */
    Color color{230, 230, 230, 255}; /**< Text color */
};

#endif /* !COMPONENTS_HPP_ */
