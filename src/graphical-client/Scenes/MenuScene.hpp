/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** MenuScene - Main menu scene
*/

#pragma once

#include "IScene.hpp"
#include "../Raylib/Raylib.hpp"
#include "../../Network/Client/NetworkClient.hpp"

class SceneManager;

/**
 * @class MenuScene
 * @brief Main menu scene with play button
 * 
 * Displays the main menu where players can:
 * - Click the play button to start the game
 * - Navigate to different game sections
 * 
 * This is an example scene demonstrating how to create
 * interactive scenes with button handling.
 */
class MenuScene : public IScene {
public:
    /**
     * @brief Constructor for MenuScene
     * @param window Reference to the Raylib window
     * @param networkClient Reference to the network client
     */
    MenuScene(Raylib::Window& window, NetworkClient& networkClient);
    
    /**
     * @brief Destructor
     */
    ~MenuScene() = default;

    // IScene interface implementation
    /**
     * @brief Called when entering the menu scene
     */
    void onEnter() override;
    
    /**
     * @brief Called when exiting the menu scene
     */
    void onExit() override;
    
    /**
     * @brief Updates menu logic (button clicks, animations, etc.)
     * @param dt Delta time since last frame
     */
    void update(float dt) override;
    
    /**
     * @brief Renders the menu interface
     * @param dt Delta time since last frame
     */
    void render(float dt) override;
    
    /**
     * @brief Sets the scene manager for scene transitions
     * @param sceneManager Pointer to the SceneManager
     */
    void setSceneManager(SceneManager* sceneManager) override { _sceneManager = sceneManager; }

private:
    // Reference members
    Raylib::Window& _window;           ///< Reference to the Raylib window
    NetworkClient& _net;               ///< Reference to the network client
    SceneManager* _sceneManager;       ///< Pointer to the scene manager (for transitions)

    // Button positions and dimensions
    int _playButtonX;                  ///< Play button X coordinate
    int _playButtonY;                  ///< Play button Y coordinate
    int _playButtonWidth;              ///< Play button width
    int _playButtonHeight;             ///< Play button height

    /**
     * @brief Checks if a mouse cursor is hovering over a button
     * @param x Button X coordinate
     * @param y Button Y coordinate
     * @param width Button width
     * @param height Button height
     * @return true if cursor is over the button, false otherwise
     */
    bool isButtonHovered(int x, int y, int width, int height);
};
