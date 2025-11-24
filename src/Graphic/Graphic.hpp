#pragma once
#include "raylib-cpp.hpp"

namespace Rtype
{
namespace Graphic
{

class AnimatedSprite
{
  private:
    raylib::Texture2D _texture;
    raylib::Vector2 _size;
    raylib::Vector2 _positionInSpritesheet;
    int _maxFrames;
    float _frameTime;
    float _timer;
    int _currentFrame;

    raylib::Rectangle _sourceRect;
    raylib::Vector2 _position;

  public:
    AnimatedSprite(const std::string &path, raylib::Vector2 size, raylib::Vector2 posInSheet, int maxFrames,
                   float frameTime = 0.1f, raylib::Vector2 pos = {0, 0})
        : _texture(path), _size(size), _positionInSpritesheet(posInSheet), _maxFrames(maxFrames), _frameTime(frameTime),
          _position(pos), _timer(0.0f), _currentFrame(0)
    {
        _sourceRect = raylib::Rectangle(posInSheet.x * size.x, posInSheet.y * size.y, size.x, size.y);
    }

    void setPosition(const raylib::Vector2 &pos)
    {
        _position = pos;
    }

    void update()
    {
        _timer += GetFrameTime();

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
        _texture.Draw(_sourceRect, _position);
    }
};

} // namespace Graphic
} // namespace Rtype
