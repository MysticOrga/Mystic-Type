/*
** EPITECH PROJECT, 2025
** client
** File description:
** Raylib Encapsulation
*/

/**
 * @file Raylib.hpp
 * @brief C++ wrapper classes for Raylib graphics library.
 *
 * Provides object-oriented interfaces for window management, texture handling,
 * drawing operations, and input detection using Raylib.
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
 * @brief C++ wrapper namespace for Raylib functionality.
 */
namespace Raylib
{
    /**
     * @class Window
     * @brief RAII wrapper for Raylib window management.
     *
     * Handles window creation, destruction, and basic graphics operations.
     */
    class Window {
    public:
        /**
         * @brief Constructor creating a window.
         * @param width Window width in pixels
         * @param height Window height in pixels
         * @param title Window title string
         */
        Window(int width, int height, const std::string &title);
        ~Window();

        /**
         * @brief Check if the window should close.
         * @return True if close requested or ESC pressed, false otherwise
         */
        bool shouldClose() const;

        /**
         * @brief Close the window.
         */
        void close();

        /**
         * @brief Begin drawing frame.
         *
         * Must be called before any drawing operations.
         */
        void beginDrawing();

        /**
         * @brief End drawing frame and display.
         *
         * Must be called after all drawing operations.
         */
        void endDrawing();

        /**
         * @brief Clear background with specified color.
         * @param color Background color
         */
        void clearBackground(Color color);

        /**
         * @brief Get window width.
         * @return Width in pixels
         */
        int getWidth() const;

        /**
         * @brief Get window height.
         * @return Height in pixels
         */
        int getHeight() const;

        /**
         * @brief Get frame time.
         * @return Time since last frame in seconds
         */
        float getFrameTime() const;

        /**
         * @brief Set target frames per second.
         * @param fps Target FPS value
         */
        void setTargetFPS(int fps);
    };

    /**
     * @class Texture
     * @brief RAII wrapper for texture loading and rendering.
     *
     * Handles texture resources, loading from file, and drawing operations.
     */
    class Texture {
    public:
        /**
         * @brief Constructor loading texture from file.
         * @param fileName Path to image file
         */
        Texture(const std::string &fileName);
        ~Texture();

        /**
         * @brief Draw texture with source rectangle to screen position.
         * @param source Source rectangle in the texture
         * @param position Screen position where to draw
         * @param tint Color tint to apply (default WHITE for no tint)
         */
        void draw(Rectangle source, Vector2 position, Color tint = WHITE);

        /**
         * @brief Draw texture with source and destination rectangles.
         * @param source Source rectangle in the texture
         * @param dest Destination rectangle on screen
         * @param tint Color tint to apply (default WHITE for no tint)
         */
        void draw(Rectangle source, Rectangle dest, Color tint = WHITE);

        /**
         * @brief Get texture width.
         * @return Width in pixels
         */
        int getWidth() const;

        /**
         * @brief Get texture height.
         * @return Height in pixels
         */
        int getHeight() const;

    private:
        Texture2D _texture; ///< Raylib Texture2D handle
    };

    /**
     * @class Draw
     * @brief Static utility class for drawing operations.
     *
     * Provides static methods for drawing shapes and text.
     */
    class Draw {
    public:
        /**
         * @brief Draw a filled circle.
         * @param centerX Circle center X coordinate
         * @param centerY Circle center Y coordinate
         * @param radius Circle radius in pixels
         * @param color Circle color
         */
        static void circle(int centerX, int centerY, float radius, Color color);

        /**
         * @brief Draw a filled rectangle.
         * @param posX Rectangle top-left X coordinate
         * @param posY Rectangle top-left Y coordinate
         * @param width Rectangle width in pixels
         * @param height Rectangle height in pixels
         * @param color Rectangle fill color
         */
        static void rectangle(int posX, int posY, int width, int height, Color color);

        /**
         * @brief Draw rectangle outline.
         * @param posX Rectangle top-left X coordinate
         * @param posY Rectangle top-left Y coordinate
         * @param width Rectangle width in pixels
         * @param height Rectangle height in pixels
         * @param color Rectangle outline color
         */
        static void rectangleLines(int posX, int posY, int width, int height, Color color);

        /**
         * @brief Draw text string.
         * @param msg Text message to draw
         * @param posX Text top-left X coordinate
         * @param posY Text top-left Y coordinate
         * @param fontSize Font size in pixels
         * @param color Text color
         */
        static void text(const std::string &msg, int posX, int posY, int fontSize, Color color);
    };

    /**
     * @class Input
     * @brief Static utility class for input detection.
     *
     * Provides static methods for keyboard input polling.
     */
    class Input {
    public:
        /**
         * @brief Check if a key is currently held down.
         * @param key Key code to check
         * @return True if key is held, false otherwise
         */
        static bool isKeyDown(int key);

        /**
         * @brief Check if a key was just pressed this frame.
         * @param key Key code to check
         * @return True if key was pressed, false otherwise
         */
        static bool isKeyPressed(int key);
    };

} // namespace Raylib

#endif /* !RAYLIB_HPP_ */
