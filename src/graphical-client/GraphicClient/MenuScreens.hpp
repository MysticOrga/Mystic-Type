/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** MenuScreens
*/

#pragma once

#include "../Raylib/Raylib.hpp"
#include "MenuState.hpp"
#include "../ecs/Components.hpp"
#include "../ecs/Core.hpp"
#include "../../Network/Client/NetworkClient.hpp"
#include <string>
#include <vector>
#include <chrono>

class MenuScreens {
public:
    MenuScreens(Raylib::Window& window) : _window(window) {}
    ~MenuScreens() = default;

    // Screen for server connection
    bool showConnectionScreen(MenuState& menuState, NetworkClient& net);
    
    // Screen for settings
    void showSettingsScreen(MenuState& menuState);

private:
    Raylib::Window& _window;
    float _hoverAnimTimer = 0.0f;
    
    void drawTitle(const std::string& title);
    void drawSubtitle(const std::string& subtitle, float y);
    void drawInputField(UIInput& input, float dt, Vector2 mousePos, bool click);
    void drawButton(UIButton& button, Vector2 mousePos, bool click, float hoverAnimTimer);
    void updateInputField(UIInput& input, float dt);
};
