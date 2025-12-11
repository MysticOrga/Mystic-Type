/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** LobbyScene - Player lobby scene (Example)
*/

#include "LobbyScene.hpp"
#include "SceneManager.hpp"
#include <iostream>

LobbyScene::LobbyScene(Raylib::Window& window, NetworkClient& networkClient)
    : _window(window), _net(networkClient)
{
    _startButtonX = 1920 / 2 - _startButtonWidth / 2;
    _startButtonY = 950;
}

void LobbyScene::onEnter()
{
    std::cout << "[LobbyScene] Entering lobby scene\n";
    updatePlayerList();
}

void LobbyScene::onExit()
{
    std::cout << "[LobbyScene] Exiting lobby scene\n";
    _playerNames.clear();
}

void LobbyScene::updatePlayerList()
{
    _playerNames.clear();
    // Fetch player list from network state
    // Example: for (const auto& player : _net.getPlayerList())
    //     _playerNames.push_back(player.name);
}

bool LobbyScene::isButtonHovered(int x, int y, int width, int height)
{
    // Note: GetMouseX() and GetMouseY() are Raylib functions
    // Uncomment if your Raylib wrapper includes these
    // int mouseX = GetMouseX();
    // int mouseY = GetMouseY();
    // return mouseX >= x && mouseX <= x + width &&
    //        mouseY >= y && mouseY <= y + height;
    return false;
}

void LobbyScene::update(float dt)
{
    // Update lobby logic
    // Refresh player list periodically
    // Check for start button click, etc.

    // Example: Check if start button is clicked
    // if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && isButtonHovered(_startButtonX, _startButtonY, _startButtonWidth, _startButtonHeight)) {
    //     sceneManager->changeScene("game");
    // }
}

void LobbyScene::render(float dt)
{
    _window.beginDrawing();
    _window.clearBackground(RAYWHITE);

    // Draw lobby title
    // DrawText("LOBBY", 1920/2 - 75, 50, 50, DARKBLUE);

    // Draw player count
    // DrawText(("Players: " + std::to_string(_playerNames.size())).c_str(), 100, 150, 30, BLACK);

    // Draw player list
    // int yOffset = 200;
    // for (const auto& playerName : _playerNames) {
    //     DrawText(playerName.c_str(), 150, yOffset, 20, BLACK);
    //     yOffset += 40;
    // }

    // Draw start button
    // bool hovered = isButtonHovered(_startButtonX, _startButtonY, _startButtonWidth, _startButtonHeight);
    // Color buttonColor = hovered ? DARKBLUE : BLUE;
    // DrawRectangle(_startButtonX, _startButtonY, _startButtonWidth, _startButtonHeight, buttonColor);
    // DrawText("START", _startButtonX + 50, _startButtonY + 10, 30, WHITE);

    _window.endDrawing();
}
