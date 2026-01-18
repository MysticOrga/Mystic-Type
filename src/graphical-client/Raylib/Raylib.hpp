/*
** EPITECH PROJECT, 2025
** client
** File description:
** Raylib Encapsulation
*/

#ifndef RAYLIB_HPP_
#define RAYLIB_HPP_

#ifdef _WIN32
    #undef Rectangle
    #undef ShowCursor
    #undef CloseWindow
#endif
#include <raylib.h>
#include <string>

/**
* @namespace Raylib
* @brief Wrapper namespace for Raylib graphics library.
*
* Provides simplified interfaces for window management, texture handling,
* drawing operations, and input handling.
*/
namespace Raylib
{
    /**
    * @class Window
    * @brief Encapsulates Raylib window management.
    *
    * Handles window creation, rendering, and lifecycle.
    */
    class Window {
    public:
        /**
        * @brief Creates a Raylib window.
        * @param width Window width in pixels.
        * @param height Window height in pixels.
        * @param title Window title string.
        */
        Window(int width, int height, const std::string &title);
        ~Window();

        /**
        * @brief Checks if window should close.
        * @return True if close was requested, false otherwise.
        */
        bool shouldClose() const;
        
        /**
        * @brief Closes the window.
        */
        void close();
        
        /**
        * @brief Begins drawing frame.
        */
        void beginDrawing();
        
        /**
        * @brief Ends drawing frame and displays it.
        */
        void endDrawing();
        
        /**
        * @brief Clears background with specified color.
        * @param color Clear color.
        */
        void clearBackground(Color color);
        
        /**
        * @brief Gets window width.
        * @return Width in pixels.
        */
        int getWidth() const;
        
        /**
        * @brief Gets window height.
        * @return Height in pixels.
        */
        int getHeight() const;
        
        /**
        * @brief Gets time delta since last frame.
        * @return Delta time in seconds.
        */
        float getFrameTime() const;
        
        /**
        * @brief Sets target FPS (frames per second).
        * @param fps Target framerate.
        */
        void setTargetFPS(int fps);
    };

    /**
    * @class Texture
    * @brief Encapsulates Raylib texture management.
    *
    * Loads and manages 2D texture drawing.
    */
    class Texture {
    public:
        /**
        * @brief Loads a texture from file.
        * @param fileName Path to the image file.
        */
        Texture(const std::string &fileName);
        ~Texture();

        /**
        * @brief Draws texture with source and destination.
        * @param source Source rectangle in texture.
        * @param position Destination position on screen.
        * @param tint Color tint (default: WHITE).
        */
        void draw(Rectangle source, Vector2 position, Color tint = WHITE);
        
        /**
        * @brief Draws texture with source and destination rectangles.
        * @param source Source rectangle in texture.
        * @param dest Destination rectangle on screen.
        * @param tint Color tint (default: WHITE).
        */
        void draw(Rectangle source, Rectangle dest, Color tint = WHITE);
        
        /**
        * @brief Gets texture width.
        * @return Width in pixels.
        */
        int getWidth() const;
        
        /**
        * @brief Gets texture height.
        * @return Height in pixels.
        */
        int getHeight() const;

    private:
        Texture2D _texture; /**< Raylib texture2D object */
    };

    /**
    * @class Draw
    * @brief Static drawing utilities.
    *
    * Provides drawing functions for basic shapes and text.
    */
    class Draw {
    public:
        /**
        * @brief Draws a filled circle.
        * @param centerX Center X coordinate.
        * @param centerY Center Y coordinate.
        * @param radius Circle radius.
        * @param color Fill color.
        */
        static void circle(int centerX, int centerY, float radius, Color color);
        
        /**
        * @brief Draws a filled rectangle.
        * @param posX X position.
        * @param posY Y position.
        * @param width Rectangle width.
        * @param height Rectangle height.
        * @param color Fill color.
        */
        static void rectangle(int posX, int posY, int width, int height, Color color);
        
        /**
        * @brief Draws a rectangle outline.
        * @param posX X position.
        * @param posY Y position.
        * @param width Rectangle width.
        * @param height Rectangle height.
        * @param color Outline color.
        */
        static void rectangleLines(int posX, int posY, int width, int height, Color color);
        
        /**
        * @brief Draws text on screen.
        * @param msg Text message.
        * @param posX X position.
        * @param posY Y position.
        * @param fontSize Font size.
        * @param color Text color.
        */
        static void text(const std::string &msg, int posX, int posY, int fontSize, Color color);
    };

    /**
    * @class Input
    * @brief Static input handling utilities.
    *
    * Provides keyboard input detection functions.
    */
    class Input {
    public:
        /**
        * @brief Checks if a key is currently pressed down.
        * @param key Raylib key code.
        * @return True if key is held down, false otherwise.
        */
        static bool isKeyDown(int key);
        
        /**
        * @brief Checks if a key was just pressed this frame.
        * @param key Raylib key code.
        * @return True if key was just pressed, false otherwise.
        */
        static bool isKeyPressed(int key);
    };

} // namespace Raylib

#endif /* !RAYLIB_HPP_ */
