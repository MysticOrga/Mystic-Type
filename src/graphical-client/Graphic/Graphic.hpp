/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** Graphic
*/

/**
 * @file Graphic.hpp
 * @brief Graphics and sprite rendering classes.
 *
 * Contains sprite animation and rendering functionality using Raylib.
 */

#pragma once
#include "../Raylib/Raylib.hpp"
#include <memory>
#include <string>

/**
 * @namespace Rtype
 * @brief Root namespace for the R-Type project.
 */
namespace Rtype
{
/**
 * @namespace Rtype::Graphic
 * @brief Graphics-related classes and utilities.
 */
namespace Graphic
{

/**
 * @class AnimatedSprite
 * @brief Handles animated sprite rendering with frame management.
 *
 * This class manages sprite animation by handling frame timing, sprite sheet
 * positioning, and rendering to the screen with support for scaling and positioning.
 */
class AnimatedSprite
{
  private:
    std::shared_ptr<Raylib::Texture> _texture; ///< Sprite sheet texture
    Vector2 _size;                             ///< Size of each frame
    Vector2 _positionInSpritesheet;            ///< Starting row/column in sprite sheet
    int _maxFrames;                            ///< Total number of animation frames
    float _frameTime;                          ///< Duration of each frame in seconds
    float _timer;                              ///< Current frame timer
    int _currentFrame;                         ///< Currently displayed frame index
    Rectangle _sourceRect;                     ///< Source rectangle in sprite sheet
    Vector2 _position;                         ///< Screen position
    Vector2 _scale{1.0f, 1.0f};               ///< Scale factors

  public:
    /**
     * @brief Constructor for AnimatedSprite.
     *
     * @param path Path to the sprite sheet image file
     * @param size Size of each frame (width, height)
     * @param posInSheet Starting position in the sprite sheet (row, column)
     * @param maxFrames Total number of frames in the animation
     * @param frameTime Duration of each frame in seconds (default 0.1)
     * @param pos Initial screen position (default {0, 0})
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
     * @brief Set the screen position of the sprite.
     * @param pos New screen position
     */
    void setPosition(const Vector2 &pos)
    {
        _position = pos;
    }

    /**
     * @brief Set the scale factors for the sprite.
     * @param sx Scale factor for X axis
     * @param sy Scale factor for Y axis
     */
    void setScale(float sx, float sy)
    {
        _scale = {sx, sy};
    }

    /**
     * @brief Update sprite animation frame based on elapsed time.
     *
     * Advances the animation frame if enough time has passed.
     * Loops back to the first frame when the last frame is reached.
     *
     * @param dt Delta time since last update in seconds
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
     * @brief Draw the sprite to the screen.
     *
     * Renders the current frame of the animation at the specified position and scale.
     * The sprite is centered on its position.
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
