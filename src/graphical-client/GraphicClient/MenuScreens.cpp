/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** MenuScreens implementation
*/

#include "graphical-client/GraphicClient/MenuScreens.hpp"
#include "graphical-client/Raylib/Raylib.hpp"
#include <cctype>
#include <cmath>
#include <iostream>

void MenuScreens::drawTitle(const std::string& title)
{
    const float screenWidth = 1920.0f;
    int titleSize = 60;
    int titleWidth = MeasureText(title.c_str(), titleSize);
    float titleX = (screenWidth - titleWidth) / 2.0f;

    for (int i = 3; i > 0; i--)
    {
        Color glowColor = {100, 150, 255, static_cast<unsigned char>(50 - i * 10)};
        Raylib::Draw::text(title.c_str(), static_cast<int>(titleX - i), 80 - i, titleSize, glowColor);
        Raylib::Draw::text(title.c_str(), static_cast<int>(titleX + i), 80 + i, titleSize, glowColor);
    }
    Raylib::Draw::text(title.c_str(), static_cast<int>(titleX), 80, titleSize, {100, 200, 255, 255});
}

void MenuScreens::drawSubtitle(const std::string& subtitle, float y)
{
    const float screenWidth = 1920.0f;
    int subtitleWidth = MeasureText(subtitle.c_str(), 20);
    Raylib::Draw::text(subtitle.c_str(), static_cast<int>((screenWidth - subtitleWidth) / 2.0f), 
                       static_cast<int>(y), 20, {150, 200, 255, 200});
}

void MenuScreens::updateInputField(UIInput& input, float dt)
{
    if (input.focused)
    {
        input.blinkTimer += dt;
        
        int key = GetCharPressed();
        while (key > 0)
        {
            if (key == '\n' || key == '\r')
            {
                input.focused = false;
            }
            else if ((std::isalnum(key) || key == '.' || key == '_' || key == '-') && 
                     input.value.size() < static_cast<size_t>(input.maxLength))
            {
                input.value.push_back(static_cast<char>(key));
            }
            key = GetCharPressed();
        }
        
        if (Raylib::Input::isKeyPressed(KEY_BACKSPACE) && !input.value.empty())
            input.value.pop_back();
    }
}

void MenuScreens::drawInputField(UIInput& input, float dt, Vector2 mousePos, bool click)
{
    updateInputField(input, dt);
    
    if (click && CheckCollisionPointRec(mousePos, input.bounds))
    {
        input.focused = true;
        input.blinkTimer = 0.0f;
    }
    
    Color borderColor = input.focused ? input.focusedBorderColor : input.borderColor;
    Raylib::Draw::rectangleLines(static_cast<int>(input.bounds.x) - 2, 
                                 static_cast<int>(input.bounds.y) - 2,
                                 static_cast<int>(input.bounds.width) + 4, 
                                 static_cast<int>(input.bounds.height) + 4, borderColor);
    
    Raylib::Draw::rectangle(static_cast<int>(input.bounds.x), 
                           static_cast<int>(input.bounds.y),
                           static_cast<int>(input.bounds.width), 
                           static_cast<int>(input.bounds.height), input.bgColor);
    
    std::string display = input.value;
    if (input.focused && static_cast<int>(input.blinkTimer * 2.0f) % 2 == 0)
        display.push_back('_');
    
    std::string textToDisplay = display.empty() ? input.placeholder : display;
    Color textColor = display.empty() ? Color{100, 150, 200, 150} : input.textColor;
    
    int textW = MeasureText(textToDisplay.c_str(), 24);
    Raylib::Draw::text(textToDisplay.c_str(), 
                      static_cast<int>(input.bounds.x + 15),
                      static_cast<int>(input.bounds.y + 18), 24, textColor);
    
    if (!input.label.empty())
    {
        Raylib::Draw::text(input.label.c_str(), 
                          static_cast<int>(input.bounds.x),
                          static_cast<int>(input.bounds.y - 30), 18, {150, 200, 255, 255});
    }
}

void MenuScreens::drawButton(UIButton& button, Vector2 mousePos, bool click, float hoverAnimTimer)
{
    button.hovered = CheckCollisionPointRec(mousePos, button.bounds);

    Color borderColor = button.hovered ? Color{100, 200, 255, 200} : Color{50, 150, 200, 150};
    Color bgColor = button.hovered ? Color{30, 80, 150, 220} : Color{20, 50, 100, 180};

    Raylib::Draw::rectangleLines(static_cast<int>(button.bounds.x) - 2, 
                                static_cast<int>(button.bounds.y) - 2,
                                static_cast<int>(button.bounds.width) + 4,
                                static_cast<int>(button.bounds.height) + 4, borderColor);

    Raylib::Draw::rectangle(static_cast<int>(button.bounds.x), 
                           static_cast<int>(button.bounds.y),
                           static_cast<int>(button.bounds.width), 
                           static_cast<int>(button.bounds.height), bgColor);

    if (button.hovered)
    {
        Raylib::Draw::rectangleLines(static_cast<int>(button.bounds.x) + 1, 
                                    static_cast<int>(button.bounds.y) + 1,
                                    static_cast<int>(button.bounds.width) - 2,
                                    static_cast<int>(button.bounds.height) - 2, 
                                    Color{150, 220, 255, 100});

        float phase = std::sin(hoverAnimTimer * 3.0f) * 2.0f + 3.0f;
        Raylib::Draw::rectangleLines(
            static_cast<int>(button.bounds.x) - static_cast<int>(phase),
            static_cast<int>(button.bounds.y) - static_cast<int>(phase),
            static_cast<int>(button.bounds.width) + static_cast<int>(phase) * 2,
            static_cast<int>(button.bounds.height) + static_cast<int>(phase) * 2,
            Color{100, 200, 255, static_cast<unsigned char>(100 + 55.0f * std::sin(hoverAnimTimer * 4.0f))});
    }

    int textW = MeasureText(button.label.c_str(), 24);
    Color textColor = button.hovered ? Color{255, 255, 255, 255} : Color{180, 220, 255, 220};
    Raylib::Draw::text(button.label.c_str(), 
                      static_cast<int>(button.bounds.x + (button.bounds.width - textW) / 2.0f),
                      static_cast<int>(button.bounds.y + (button.bounds.height - 24) / 2.0f), 
                      24, textColor);
}

bool MenuScreens::showConnectionScreen(MenuState& menuState, NetworkClient& net)
{
    const float screenWidth = 1920.0f;
    const float screenHeight = 1080.0f;
    
    const float inputWidth = 300.0f;
    const float inputHeight = 60.0f;
    const float centerX = (screenWidth - inputWidth) / 2.0f;
    const float startY = 280.0f;
    const float spacing = 110.0f;
    
    UIInput ipInput;
    ipInput.bounds = {centerX, startY, inputWidth, inputHeight};
    ipInput.label = "SERVER IP:";
    ipInput.value = menuState.getServerIp();
    ipInput.placeholder = "127.0.0.1";
    ipInput.maxLength = 15;
    
    UIInput portInput;
    portInput.bounds = {centerX, startY + spacing, inputWidth, inputHeight};
    portInput.label = "SERVER PORT:";
    portInput.value = std::to_string(menuState.getServerPort());
    portInput.placeholder = "4243";
    portInput.maxLength = 5;
    
    const float buttonWidth = 280.0f;
    const float buttonHeight = 70.0f;
    const float buttonCenterX = (screenWidth - buttonWidth) / 2.0f;
    const float playButtonY = startY + spacing * 2.5f;
    
    UIButton playButton;
    playButton.bounds = {buttonCenterX, playButtonY, buttonWidth, buttonHeight};
    playButton.label = "PLAY";
    playButton.action = "play";
    
    UIButton settingsButton;
    settingsButton.bounds = {buttonCenterX, playButtonY + 100, buttonWidth, buttonHeight};
    settingsButton.label = "SETTINGS";
    settingsButton.action = "settings";
    
    bool submitted = false;
    bool goToSettings = false;
    float hoverAnimTimer = 0.0f;
    std::string errorMessage;
    float errorTimer = 0.0f;
    
    while (!submitted && !goToSettings && !_window.shouldClose())
    {
        float dt = _window.getFrameTime();
        hoverAnimTimer += dt;
        if (!errorMessage.empty())
            errorTimer -= dt;
        if (errorTimer < 0)
            errorMessage.clear();
        
        Vector2 mousePos = {static_cast<float>(GetMouseX()), static_cast<float>(GetMouseY())};
        bool click = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
        
        _window.beginDrawing();
        _window.clearBackground({15, 25, 50, 255});
        
        drawTitle("MYSTIC TYPE");
        drawSubtitle("SERVER CONNECTION", 155.0f);
        
        drawInputField(ipInput, dt, mousePos, click);
        drawInputField(portInput, dt, mousePos, click);
        
        drawButton(playButton, mousePos, false, hoverAnimTimer);
        drawButton(settingsButton, mousePos, false, hoverAnimTimer);
        
        if (click)
        {
            if (CheckCollisionPointRec(mousePos, playButton.bounds))
            {
                submitted = true;
            }
            else if (CheckCollisionPointRec(mousePos, settingsButton.bounds))
            {
                goToSettings = true;
            }
        }
        
        // Validate inputs on play button press
        if (submitted)
        {
            if (ipInput.value.empty())
            {
                submitted = false;
                errorMessage = "Entrez une adresse IP";
                errorTimer = 2.0f;
            }
            else if (portInput.value.empty())
            {
                submitted = false;
                errorMessage = "Entrez un port";
                errorTimer = 2.0f;
            }
            else
            {
                try {
                    int port = std::stoi(portInput.value);
                    if (port < 1 || port > 65535)
                    {
                        submitted = false;
                        errorMessage = "Port invalide (1-65535)";
                        errorTimer = 2.0f;
                    }
                    else
                    {
                        menuState.setServerIp(ipInput.value);
                        menuState.setServerPort(port);
                    }
                } catch (...) {
                    submitted = false;
                    errorMessage = "Port invalide";
                    errorTimer = 2.0f;
                }
            }
        }
        
        // Display error message
        if (!errorMessage.empty())
        {
            int errorWidth = MeasureText(errorMessage.c_str(), 20);
            Raylib::Draw::text(errorMessage.c_str(), 
                              static_cast<int>((screenWidth - errorWidth) / 2.0f),
                              static_cast<int>(playButtonY + 130), 20, {255, 100, 100, 255});
        }
        
        // Display instructions
        Raylib::Draw::text("Press Tab to switch inputs", 
                          static_cast<int>(centerX), 
                          static_cast<int>(startY - 50), 16, {150, 200, 255, 200});
        
        _window.endDrawing();
        
        if (goToSettings)
        {
            showSettingsScreen(menuState);
            goToSettings = false;
            submitted = false;
        }
    }
    
    return submitted;
}

void MenuScreens::showSettingsScreen(MenuState& menuState)
{
    const float screenWidth = 1920.0f;
    const float screenHeight = 1080.0f;
    
    bool exitSettings = false;
    float hoverAnimTimer = 0.0f;
    
    const float buttonWidth = 280.0f;
    const float buttonHeight = 70.0f;
    const float buttonCenterX = (screenWidth - buttonWidth) / 2.0f;
    const float backButtonY = 600.0f;
    
    UIButton backButton;
    backButton.bounds = {buttonCenterX, backButtonY, buttonWidth, buttonHeight};
    backButton.label = "BACK";
    backButton.action = "back";
    
    while (!exitSettings && !_window.shouldClose())
    {
        float dt = _window.getFrameTime();
        hoverAnimTimer += dt;
        
        Vector2 mousePos = {static_cast<float>(GetMouseX()), static_cast<float>(GetMouseY())};
        bool click = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
        
        _window.beginDrawing();
        _window.clearBackground({15, 25, 50, 255});
        
        drawTitle("SETTINGS");
        drawSubtitle("Configuration de jeu", 155.0f);
        
        // Display placeholder settings message
        const char* settingsText = "Les parametres seront ajoutes ici";
        int textWidth = MeasureText(settingsText, 24);
        Raylib::Draw::text(settingsText, 
                          static_cast<int>((screenWidth - textWidth) / 2.0f),
                          300, 24, {150, 200, 255, 200});
        
        drawButton(backButton, mousePos, click, hoverAnimTimer);
        
        if (click && CheckCollisionPointRec(mousePos, backButton.bounds))
        {
            exitSettings = true;
        }
        
        _window.endDrawing();
    }
}
