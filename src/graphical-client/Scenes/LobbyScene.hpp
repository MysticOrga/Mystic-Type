/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** LobbyScene - Player lobby scene
*/

#pragma once

#include "IScene.hpp"
#include "../Raylib/Raylib.hpp"
#include "../../Network/Client/NetworkClient.hpp"
#include <vector>
#include <string>

class SceneManager;

/**
 * @class LobbyScene
 * @brief Player lobby scene for waiting and game start
 * 
 * The lobby scene displays:
 * - List of connected players
 * - Player count
 * - Start game button
 * - Ready status indicators
 * 
 * Players wait in the lobby before the game starts,
 * and can see who else is connected.
 */
class LobbyScene : public IScene {
public:
    /**
     * @brief Constructor for LobbyScene
     * @param window Reference to the Raylib window
     * @param networkClient Reference to the network client
     */
    LobbyScene(Raylib::Window& window, NetworkClient& networkClient);
    
    /**
     * @brief Destructor
     */
    ~LobbyScene() = default;

    // IScene interface implementation
    /**
     * @brief Called when entering the lobby scene
     */
    void onEnter() override;
    
    /**
     * @brief Called when exiting the lobby scene
     */
    void onExit() override;
    
    /**
     * @brief Updates lobby logic
     * @param dt Delta time since last frame
     */
    void update(float dt) override;
    
    /**
     * @brief Renders the lobby interface
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

    // Player list
    std::vector<std::string> _playerNames;  ///< List of connected player names

    // Button positions
    int _startButtonX;                 ///< Start button X coordinate
    int _startButtonY;                 ///< Start button Y coordinate
    int _startButtonWidth;             ///< Start button width
    int _startButtonHeight;            ///< Start button height

    /**
     * @brief Updates the player list from network state
     */
    void updatePlayerList();
    
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
