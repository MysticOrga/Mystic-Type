/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** Graphic
*/

#pragma once
#include "../Raylib/Raylib.hpp"
#include <memory>
#include <string>

/**
* @namespace Rtype
* @brief Main namespace for R-Type game.
*/
namespace Rtype
{
/**
* @namespace Graphic
* @brief Graphics-related classes and utilities.
*/
namespace Graphic
{

/**
* @class AnimatedSprite
* @brief Manages animated sprite rendering with frame-based animation.
*
* Handles sprite sheet loading, animation frame updates, positioning, and rendering.
*/
class AnimatedSprite
{
  private:
    std::shared_ptr<Raylib::Texture> _texture; /**< Loaded sprite sheet texture */

    Vector2 _size;                  /**< Size of each individual frame */
    Vector2 _positionInSpritesheet; /**< Starting position in sprite sheet */
    int _maxFrames;                 /**< Total number of animation frames */
    float _frameTime;               /**< Time each frame is displayed */
    float _timer;                   /**< Current frame timer */
    int _currentFrame;              /**< Currently displayed frame index */

    Rectangle _sourceRect;  /**< Source rectangle in sprite sheet */
    Vector2 _position;      /**< Current screen position */
    Vector2 _scale{1.0f, 1.0f}; /**< Sprite scale factors */

  public:
    /**
    * @brief Constructor for AnimatedSprite.
    * @param path Path to sprite sheet image file.
    * @param size Size of each frame (width, height).
    * @param posInSheet Position of first frame in sprite sheet grid.
    * @param maxFrames Total number of animation frames.
    * @param frameTime Time in seconds each frame is displayed (default: 0.1).
    * @param pos Initial screen position (default: 0, 0).
    */
    AnimatedSprite(const std::string &path, Vector2 size, Vector2 posInSheet, int maxFrames,
                   float frameTime = 0.1f, Vector2 pos = {0, 0})
        : _size(size), _positionInSpritesheet(posInSheet), _maxFrames(maxFrames), _frameTime(frameTime),
          _position(pos), _timer(0.0f), _currentFrame(0)
    {
        _texture = std::make_shared<Raylib::Texture>(path);

        _sourceRect = {
            posInSheet.x * size.x, 
            posInSheet.y * size.y, 
            size.x, 
            size.y
        };
    }

    /**
    * @brief Sets the sprite's screen position.
    * @param pos New position coordinates.
    */
    void setPosition(const Vector2 &pos)
    {
        _position = pos;
    }

    /**
    * @brief Sets the sprite's scale factors.
    * @param sx Horizontal scale.
    * @param sy Vertical scale.
    */
    void setScale(float sx, float sy)
    {
        _scale = {sx, sy};
    }

    /**
    * @brief Updates animation frame based on elapsed time.
    * @param dt Delta time since last update.
    */
    void update(float dt)
    {
        _timer += dt;

        if (_timer >= _frameTime)
        {
            _timer = 0.0f;
            _currentFrame++;

            if (_currentFrame >= _maxFrames)
                _currentFrame = 0;

            _sourceRect.x = (_positionInSpritesheet.x + _currentFrame) * _size.x;
        }
    }

    /**
    * @brief Renders the sprite to the screen.
    *
    * Draws the current animation frame centered at the sprite position.
    */
    void draw()
    {
        if (_texture) {
            float w = _sourceRect.width * _scale.x;
            float h = _sourceRect.height * _scale.y;
            Vector2 centeredPos = _position;
            centeredPos.x -= w / 2.0f;
            centeredPos.y -= h / 2.0f;

            Rectangle dest{centeredPos.x, centeredPos.y, w, h};
            _texture->draw(_sourceRect, dest, WHITE);
        }
    }
};

} // namespace Graphic
} // namespace Rtype
