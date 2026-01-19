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

/**
 * @enum KeyEvent
 * @brief Enumerates keyboard events for player input mapping.
 *
 * Maps directional and action inputs to keyboard keys.
 */
enum KeyEvent
{
    RIGHT, /**< Move right action */
    LEFT,  /**< Move left action */
    UP,    /**< Move up action */
    DOWN,  /**< Move down action */
    SHOOT  /**< Shoot action */
};

/**
 * @class InputSystem
 * @brief Handles player input and keyboard key binding management.
 *
 * This system manages keyboard input, detects key presses/holds,
 * and communicates player movement and shooting commands to the server.
 * Players can configure custom key bindings for all actions.
 */
class InputSystem
{

  public:
    /**
     * @brief Constructor initializing default key bindings.
     *
     * Sets up default arrow keys for movement and A for shooting.
     */
    InputSystem()
    {
        _keyMap[RIGHT] = KEY_RIGHT;
        _keyMap[LEFT] = KEY_LEFT;
        _keyMap[UP] = KEY_UP;
        _keyMap[DOWN] = KEY_DOWN;
        _keyMap[SHOOT] = KEY_SPACE;
    };

    /**
     * @brief Sets a custom key binding for a specific action.
     * @param event The KeyEvent action to rebind.
     * @param key The KeyboardKey to bind to this action.
     */
    inline void setKey(KeyEvent event, KeyboardKey key)
    {
        _keyMap[event] = key;
    };

    /**
     * @brief Retrieves the current key binding for an action.
     * @param event The KeyEvent action to query.
     * @return The KeyboardKey currently bound to this action.
     */
    inline KeyboardKey getKey(KeyEvent event) const
    {
        auto it = _keyMap.find(event);
        return it != _keyMap.end() ? it->second : KEY_NULL;
    };

    /**
     * @brief Updates input state and sends player commands to the server.
     *
     * Processes keyboard input for movement and shooting, applies velocity changes,
     * and sends commands to the server for validation and propagation to other players.
     *
     * @param net Reference to NetworkClient for sending commands to server.
     * @param pos Current player position.
     * @param vel Reference to velocity component (modified by this method).
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
    NetworkClient::MoveCmd _lastDir = NetworkClient::MoveCmd::Right;
    std::unordered_map<uint8_t, KeyboardKey> _keyMap;
};

/**
 * @class MovementSystem
 * @brief Handles entity position updates based on velocity.
 *
 * Updates entity positions using a fixed tick duration (32ms) to match server updates
 * and prevent frame-rate dependent velocity accumulation.
 */
class MovementSystem
{
  public:
    /**
     * @brief Updates entity position based on velocity with fixed time stepping.
     * @param ecs Reference to the ECS system.
     * @param e The entity to update.
     * @param dt Delta time since last frame.
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
    std::unordered_map<Entity, float> _accumulator;
};

/**
 * @class CircleRenderSystem
 * @brief Renders circle-based entities.
 *
 * Used for rendering simple circular game objects.
 */
class CircleRenderSystem
{
  public:
    /**
     * @brief Renders a circle entity to the screen.
     * @param ecs Reference to the ECS system.
     * @param e The entity to render.
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
 * @brief Renders rectangle-based entities with proper coordinate transformation.
 *
 * Transforms game coordinates (0-255) to screen coordinates based on game area offset and size.
 */
class RectangleRenderSystem
{
  public:
    /**
     * @brief Sets the game area offset and size for coordinate transformation.
     * @param offsetX X offset of the game area on screen.
     * @param offsetY Y offset of the game area on screen.
     * @param areaSize Size of the game area in pixels.
     */
    void setGameAreaOffset(float offsetX, float offsetY, float areaSize)
    {
        _offsetX = offsetX;
        _offsetY = offsetY;
        _areaSize = areaSize;
    }

    /**
     * @brief Renders a rectangle entity to the screen.
     * @param ecs Reference to the ECS system.
     * @param e The entity to render.
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
    float _offsetX = 0.0f;
    float _offsetY = 0.0f;
    float _areaSize = 1280.0f;
};

/**
 * @class SpriteRenderSystem
 * @brief Renders animated sprite entities with proper coordinate transformation.
 *
 * Handles drawing of animated sprites with scaling and converts game coordinates to screen coordinates.
 */
class SpriteRenderSystem
{
  public:
    /**
     * @brief Sets the sprite scaling factors.
     * @param sx Horizontal scale factor.
     * @param sy Vertical scale factor.
     */
    void setScale(float sx, float sy)
    {
        _scaleX = sx;
        _scaleY = sy;
    }

    /**
     * @brief Sets the game area offset and size for coordinate transformation.
     * @param offsetX X offset of the game area on screen.
     * @param offsetY Y offset of the game area on screen.
     * @param areaSize Size of the game area in pixels.
     */
    void setGameAreaOffset(float offsetX, float offsetY, float areaSize)
    {
        _offsetX = offsetX;
        _offsetY = offsetY;
        _areaSize = areaSize;
    }

    /**
     * @brief Updates and renders an animated sprite entity.
     * @param ecs Reference to the ECS system.
     * @param e The entity to render.
     * @param dt Delta time since last frame for animation updates.
     */
    void update(ECS &ecs, Entity e, float dt)
    {
        auto &pos = ecs.getComponent<Position>(e);
        auto &sprite = ecs.getComponent<Sprite>(e);

        if (!sprite.sprite)
            return;

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
