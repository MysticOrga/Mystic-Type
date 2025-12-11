/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** MenuScene - Main menu scene (Example)
*/

#include "MenuScene.hpp"
#include "SceneManager.hpp"
#include <iostream>

MenuScene::MenuScene(Raylib::Window& window, NetworkClient& networkClient)
    : _window(window), _net(networkClient)
{
    _playButtonX = 1920 / 2 - _playButtonWidth / 2;
    _playButtonY = 1080 / 2 - _playButtonHeight / 2;
}

void MenuScene::onEnter()
{
    std::cout << "[MenuScene] Entering menu scene\n";
    // Initialize resources if needed
}

void MenuScene::onExit()
{
    std::cout << "[MenuScene] Exiting menu scene\n";
    // Clean up resources if needed
}

bool MenuScene::isButtonHovered(int x, int y, int width, int height)
{
    int mouseX = GetMouseX();
    int mouseY = GetMouseY();
    return mouseX >= x && mouseX <= x + width &&
           mouseY >= y && mouseY <= y + height;
}

void MenuScene::update(float dt)
{
    // Update menu logic
    // Check for button clicks, handle navigation, etc.

    // Check if play button is clicked
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && isButtonHovered(_playButtonX, _playButtonY, _playButtonWidth, _playButtonHeight)) {
        std::cout << "[MenuScene] Play button clicked\n";
        if (_sceneManager != nullptr) {
            _sceneManager->changeScene("game");
        }
    }
}

void MenuScene::render(float dt)
{
    _window.beginDrawing();
    _window.clearBackground(RAYWHITE);

    // Draw menu title
    Raylib::Draw::text("MYSTIC-TYPE", 1920/2 - 150, 200, 50, DARKBLUE);

    // Draw play button
    bool hovered = isButtonHovered(_playButtonX, _playButtonY, _playButtonWidth, _playButtonHeight);
    Color buttonColor = hovered ? DARKBLUE : BLUE;
    DrawRectangle(_playButtonX, _playButtonY, _playButtonWidth, _playButtonHeight, buttonColor);
    DrawText("PLAY", _playButtonX + 50, _playButtonY + 10, 30, WHITE);

    _window.endDrawing();
}
