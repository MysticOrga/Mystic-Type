/*
** EPITECH PROJECT, 2025
** client
** File description:
** Raylib Encapsulation
*/

#ifndef RAYLIB_HPP_
#define RAYLIB_HPP_

#include <raylib.h>
#include <string>

namespace Raylib
{
    // ... (Window et Texture restent inchangés) ...
    class Window {
    public:
        Window(int width, int height, const std::string &title);
        ~Window();

        bool shouldClose() const;
        void close();
        void beginDrawing();
        void endDrawing();
        void clearBackground(Color color);
        int getWidth() const;
        int getHeight() const;
        float getFrameTime() const;
        void setTargetFPS(int fps);
    };

    class Texture {
    public:
        Texture(const std::string &fileName);
        ~Texture();

        void draw(Rectangle source, Vector2 position, Color tint = WHITE);
        int getWidth() const;
        int getHeight() const;

    private:
        Texture2D _texture;
    };

    class Draw {
    public:
        static void circle(int centerX, int centerY, float radius, Color color);
        static void rectangle(int posX, int posY, int width, int height, Color color);
        static void rectangleLines(int posX, int posY, int width, int height, Color color);
    };

    class Input {
    public:
        static bool isKeyDown(int key);
        static bool isKeyPressed(int key);
    };

} // namespace Raylib

#endif /* !RAYLIB_HPP_ */