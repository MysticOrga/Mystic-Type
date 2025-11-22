#pragma once
#include "raylib.h"
#include <string>

namespace Raylib {

    using Vector2 = ::Vector2;
    using Vector3 = ::Vector3;
    using Color   = ::Color;
    using Texture = ::Texture2D;

    class Core {
    public:
        void initWindow(int width, int height, const std::string& title);
        void closeWindow();
        bool windowShouldClose() const;
        void beginDrawing() const;
        void endDrawing() const;
        void clearBackground(Color color) const;
        void drawRectangle(float x, float y, float w, float h, Color color) const;
        void drawCircle(float x, float y, float radius, Color color) const;
        bool isKeyDown(int key) const;
        bool isKeyPressed(int key) const;
        void setTargetFPS(int fps) const;
        float getFrameTime() const;
    };
} // namespace Raylib
