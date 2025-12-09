/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** Graphic
*/

#pragma once
#include "../Raylib/Raylib.hpp" // Utilise notre wrapper
#include <memory>
#include <string>

namespace Rtype
{
namespace Graphic
{

class AnimatedSprite
{
  private:
    std::shared_ptr<Raylib::Texture> _texture;

    Vector2 _size;
    Vector2 _positionInSpritesheet;
    int _maxFrames;
    float _frameTime;
    float _timer;
    int _currentFrame;

    Rectangle _sourceRect;
    Vector2 _position;

  public:
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

    void setPosition(const Vector2 &pos)
    {
        _position = pos;
    }

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

    void draw()
    {
        if (_texture)
            _texture->draw(_sourceRect, _position, WHITE);
    }
};

} // namespace Graphic
} // namespace Rtype