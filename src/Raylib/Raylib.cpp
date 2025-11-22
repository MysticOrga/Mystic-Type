#include "Raylib.hpp"

namespace Raylib {
    void Core::initWindow(int width, int height, const std::string& title) {
        ::InitWindow(width, height, title.c_str());
    }

    void Core::closeWindow() {
        ::CloseWindow();
    }

    bool Core::windowShouldClose() const {
        return ::WindowShouldClose();
    }

    void Core::beginDrawing() const {
        ::BeginDrawing();
    }

    void Core::endDrawing() const {
        ::EndDrawing();
    }

    void Core::clearBackground(Color color) const {
        ::ClearBackground(color);
    }

    void Core::drawRectangle(float x, float y, float w, float h, Color color) const {
        ::DrawRectangle(x, y, w, h, color);
    }

    void Core::drawCircle(float x, float y, float radius, Color color) const {
        ::DrawCircle(x, y, radius, color);
    }

    bool Core::isKeyDown(int key) const {
        return ::IsKeyDown(key);
    }

    bool Core::isKeyPressed(int key) const {
        return ::IsKeyPressed(key);
    }

    void Core::setTargetFPS(int fps) const {
        ::SetTargetFPS(fps);
    }

    float Core::getFrameTime() const {
        return ::GetFrameTime();
    }

} // namespace Raylib
