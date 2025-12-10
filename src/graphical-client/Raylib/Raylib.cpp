/*
** EPITECH PROJECT, 2025
** client
** File description:
** Raylib Implementation
*/

#include "Raylib.hpp"

namespace Raylib
{
    Window::Window(int width, int height, const std::string &title)
    {
        ::InitWindow(width, height, title.c_str());
    }
    Window::~Window()
    {
        if (::IsWindowReady()) {
            ::CloseWindow();
        }
    }
    bool Window::shouldClose() const { return ::WindowShouldClose(); }
    void Window::close() { ::CloseWindow(); }
    void Window::beginDrawing() { ::BeginDrawing(); }
    void Window::endDrawing() { ::EndDrawing(); }
    void Window::clearBackground(Color color) { ::ClearBackground(color); }
    int Window::getWidth() const { return ::GetScreenWidth(); }
    int Window::getHeight() const { return ::GetScreenHeight(); }
    float Window::getFrameTime() const { return ::GetFrameTime(); }
    void Window::setTargetFPS(int fps) { ::SetTargetFPS(fps); }

    Texture::Texture(const std::string &fileName)
    {
        _texture = ::LoadTexture(fileName.c_str());
    }
    Texture::~Texture()
    {
        ::UnloadTexture(_texture);
    }
    void Texture::draw(Rectangle source, Vector2 position, Color tint)
    {
        ::DrawTextureRec(_texture, source, position, tint);
    }
    int Texture::getWidth() const { return _texture.width; }
    int Texture::getHeight() const { return _texture.height; }

    void Draw::circle(int centerX, int centerY, float radius, Color color)
    {
        ::DrawCircle(centerX, centerY, radius, color);
    }

    void Draw::rectangle(int posX, int posY, int width, int height, Color color)
    {
        ::DrawRectangle(posX, posY, width, height, color);
    }

    // [AJOUT] Implémentation
    void Draw::rectangleLines(int posX, int posY, int width, int height, Color color)
    {
        ::DrawRectangleLines(posX, posY, width, height, color);
    }

    bool Input::isKeyDown(int key) { return ::IsKeyDown(key); }
    bool Input::isKeyPressed(int key) { return ::IsKeyPressed(key); }

} // namespace Raylib