/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** System
*/

/**
 * @file System.hpp
 * @brief ECS systems for game logic and rendering.
 *
 * Contains implementations of various game systems including input handling,
 * movement, and rendering systems.
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

/**
 * @enum KeyEvent
 * @brief Enumeration of input events.
 */
enum KeyEvent
{
    RIGHT,  ///< Right movement input
    LEFT,   ///< Left movement input
    UP,     ///< Up movement input
    DOWN,   ///< Down movement input
    SHOOT   ///< Shoot input
};

/**
 * @class InputSystem
 * @brief Handles player input and sends commands to the network.
 *
 * Manages keyboard bindings and translates them to network commands.
 * Supports customizable key mappings.
 */
class InputSystem
{

  public:
    /**
     * @brief Constructor with default key bindings.
     *
     * Default bindings:
     * - RIGHT: KEY_RIGHT
     * - LEFT: KEY_LEFT
     * - UP: KEY_UP
     * - DOWN: KEY_DOWN
     * - SHOOT: KEY_A
     */
    InputSystem()
    {
        _keyMap[RIGHT] = KEY_RIGHT;
        _keyMap[LEFT] = KEY_LEFT;
        _keyMap[UP] = KEY_UP;
        _keyMap[DOWN] = KEY_DOWN;
        _keyMap[SHOOT] = KEY_A;
    };

    /**
     * @brief Set a key binding for a specific action.
     * @param event The action to rebind
     * @param key The keyboard key to bind
     */
    inline void setKey(KeyEvent event, KeyboardKey key)
    {
        _keyMap[event] = key;
    };

    /**
     * @brief Get the current key binding for an action.
     * @param event The action to query
     * @return The keyboard key bound to this action
     */
    inline KeyboardKey getKey(KeyEvent event) const
    {
        auto it = _keyMap.find(event);
        return it != _keyMap.end() ? it->second : KEY_NULL;
    };

    /**
     * @brief Update input state and send commands to the network.
     *
     * Processes keyboard input, updates local velocity, and sends
     * movement/shooting commands to the server.
     *
     * @param net Network client reference for sending commands
     * @param pos Current player position
     * @param vel Reference to velocity to be updated
     */
    void update(NetworkClient &net, const Position &pos, Velocity &vel)
    {
        bool moved = false;
        NetworkClient::MoveCmd cmd = NetworkClient::MoveCmd::Up;

        const int8_t speed = 5;

        vel.vx = 0;
        vel.vy = 0;

        if (Raylib::Input::isKeyDown(_keyMap[RIGHT]))
        {
            cmd = NetworkClient::MoveCmd::Right;
            vel.vx = speed;
            moved = true;
        }
        if (Raylib::Input::isKeyDown(_keyMap[LEFT]))
        {
            cmd = NetworkClient::MoveCmd::Left;
            vel.vx = -speed;
            moved = true;
        }
        if (Raylib::Input::isKeyDown(_keyMap[UP]))
        {
            cmd = NetworkClient::MoveCmd::Up;
            vel.vy = -speed;
            moved = true;
        }
        if (Raylib::Input::isKeyDown(_keyMap[DOWN]))
        {
            cmd = NetworkClient::MoveCmd::Down;
            vel.vy = speed;
            moved = true;
        }

        if (moved)
        {
            _lastDir = cmd;
            net.sendInput(static_cast<uint8_t>(pos.x), static_cast<uint8_t>(pos.y), (int8_t)vel.vx, (int8_t)vel.vy,
                          cmd);
        }

        if (Raylib::Input::isKeyPressed(_keyMap[SHOOT]))
        {
            const int8_t bvx = 2;
            const int8_t bvy = 0;
            float startX = pos.x + 4.0f;
            float startY = pos.y + 1.0f;
            net.sendShoot(static_cast<uint8_t>(std::clamp(startX, 0.0f, 255.0f)),
                          static_cast<uint8_t>(std::clamp(startY, 0.0f, 255.0f)), bvx, bvy);
        }
    }

  private:
    NetworkClient::MoveCmd _lastDir = NetworkClient::MoveCmd::Right; ///< Last direction moved
    std::unordered_map<uint8_t, KeyboardKey> _keyMap; ///< Key bindings map
};

/**
 * @class MovementSystem
 * @brief Handles entity movement and physics integration.
 *
 * Updates entity positions based on velocities using a fixed timestep
 * to match the server's 32ms tick duration.
 */
class MovementSystem
{
  public:
    /**
     * @brief Update entity position based on velocity.
     *
     * Integrates velocity over time using fixed timesteps to ensure
     * consistent movement regardless of frame rate.
     *
     * @param ecs ECS manager reference
     * @param e Entity to update
     * @param dt Delta time since last frame
     */
    void update(ECS &ecs, Entity e, float dt)
    {
        auto &pos = ecs.getComponent<Position>(e);
        auto &vel = ecs.getComponent<Velocity>(e);

        float &acc = _accumulator[e];
        acc += dt;
        constexpr float tickDuration = 0.032f;
        while (acc >= tickDuration)
        {
            pos.x += vel.vx;
            pos.y += vel.vy;
            acc -= tickDuration;
        }
    }

  private:
    std::unordered_map<Entity, float> _accumulator; ///< Time accumulator per entity
};

/**
 * @class CircleRenderSystem
 * @brief Renders circular entities.
 *
 * Simple debug rendering system for circle-based entities.
 */
class CircleRenderSystem
{
  public:
    /**
     * @brief Render a circle entity.
     * @param ecs ECS manager reference
     * @param e Entity to render
     */
    void update(ECS &ecs, Entity e)
    {
        auto &pos = ecs.getComponent<Position>(e);
        auto &circle = ecs.getComponent<CircleComponent>(e);
        Raylib::Draw::circle((int)(pos.x * 5), (int)(pos.y * 5), circle.radius, circle.color);
    }
};

/**
 * @class RectangleRenderSystem
 * @brief Renders rectangular entities with coordinate transformation.
 *
 * Converts game space coordinates (0-255) to screen space coordinates.
 * Primarily used for rendering projectiles.
 */
class RectangleRenderSystem
{
  public:
    /**
     * @brief Set the game area offset and size for coordinate transformation.
     * @param offsetX Screen X offset for the game area
     * @param offsetY Screen Y offset for the game area
     * @param areaSize Size of the game area in screen pixels
     */
    void setGameAreaOffset(float offsetX, float offsetY, float areaSize) 
    { 
        _offsetX = offsetX;
        _offsetY = offsetY;
        _areaSize = areaSize;
    }

    /**
     * @brief Render a rectangle entity.
     * @param ecs ECS manager reference
     * @param e Entity to render
     */
    void update(ECS &ecs, Entity e)
    {
        auto &pos = ecs.getComponent<Position>(e);
        auto &rect = ecs.getComponent<RectangleComponent>(e);
        
        float screenX = _offsetX + (pos.x / 255.0f) * _areaSize;
        float screenY = _offsetY + (pos.y / 255.0f) * _areaSize;
        
        Raylib::Draw::rectangle((int)screenX, (int)screenY, rect.width, rect.height, rect.color);
    }
  private:
    float _offsetX = 0.0f;    ///< Game area X offset
    float _offsetY = 0.0f;    ///< Game area Y offset
    float _areaSize = 1280.0f; ///< Game area size
};

/**
 * @class SpriteRenderSystem
 * @brief Renders animated sprite entities with coordinate transformation.
 *
 * Handles sprite animation and converts game space coordinates to screen space.
 */
class SpriteRenderSystem
{
  public:
    /**
     * @brief Set the sprite scale factors.
     * @param sx Scale factor for X axis
     * @param sy Scale factor for Y axis
     */
    void setScale(float sx, float sy) { _scaleX = sx; _scaleY = sy; }
    
    /**
     * @brief Set the game area offset and size for coordinate transformation.
     * @param offsetX Screen X offset for the game area
     * @param offsetY Screen Y offset for the game area
     * @param areaSize Size of the game area in screen pixels
     */
    void setGameAreaOffset(float offsetX, float offsetY, float areaSize)
    {
        _offsetX = offsetX;
        _offsetY = offsetY;
        _areaSize = areaSize;
    }

    /**
     * @brief Update and render a sprite entity.
     * @param ecs ECS manager reference
     * @param e Entity to render
     * @param dt Delta time for sprite animation
     */
    void update(ECS &ecs, Entity e, float dt)
    {
        auto &pos = ecs.getComponent<Position>(e);
        auto &sprite = ecs.getComponent<Sprite>(e);

        if (!sprite.sprite) return;
        
        float screenX = _offsetX + (pos.x / 255.0f) * _areaSize;
        float screenY = _offsetY + (pos.y / 255.0f) * _areaSize;
        
        sprite.sprite->setPosition({screenX, screenY});
        sprite.sprite->update(dt);
        sprite.sprite->draw();
    }

  private:
    float _scaleX = 1.0f;     ///< X scale factor
    float _scaleY = 1.0f;     ///< Y scale factor
    float _offsetX = 0.0f;    ///< Game area X offset
    float _offsetY = 0.0f;    ///< Game area Y offset
    float _areaSize = 1280.0f; ///< Game area size
};

#endif /* !SYSTEM_HPP_ */
