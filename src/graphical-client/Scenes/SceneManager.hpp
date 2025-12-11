/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** SceneManager - Manages scene transitions and updates
*/

#pragma once

#include "IScene.hpp"
#include <unordered_map>
#include <string>
#include <memory>

/**
 * @class SceneManager
 * @brief Manages scene lifecycle and transitions
 * 
 * The SceneManager is responsible for:
 * - Registering scenes with unique string identifiers
 * - Transitioning between scenes with proper lifecycle callbacks
 * - Updating and rendering the active scene each frame
 * 
 * Usage example:
 * @code
 * SceneManager manager;
 * auto gameScene = std::make_shared<GameScene>(window, net);
 * manager.registerScene("game", gameScene);
 * manager.changeScene("game");
 * 
 * // In game loop:
 * manager.update(deltaTime);
 * manager.render(deltaTime);
 * @endcode
 */
class SceneManager {
public:
    /**
     * @brief Constructs a SceneManager instance
     * 
     * Initializes with no active scene.
     */
    SceneManager() : _currentScene(nullptr) {}
    
    /**
     * @brief Destructor
     */
    ~SceneManager() = default;

    /**
     * @brief Registers a scene with a unique identifier
     * 
     * @param name Unique string identifier for the scene
     * @param scene Shared pointer to the scene implementation
     * 
     * @note If a scene with the same name already exists, it will be replaced.
     */
    void registerScene(const std::string& name, std::shared_ptr<IScene> scene);

    /**
     * @brief Transitions to a different scene
     * 
     * Calls onExit() on the current scene (if any) and onEnter() on the new scene.
     * 
     * @param name Identifier of the scene to transition to
     * @return true if the transition was successful, false if the scene doesn't exist
     */
    bool changeScene(const std::string& name);

    /**
     * @brief Updates the current active scene
     * 
     * @param dt Delta time since the last frame (in seconds)
     * 
     * @note Does nothing if no scene is currently active.
     */
    void update(float dt);

    /**
     * @brief Renders the current active scene
     * 
     * @param dt Delta time since the last frame (in seconds)
     * 
     * @note Does nothing if no scene is currently active.
     */
    void render(float dt);

    /**
     * @brief Gets the currently active scene
     * 
     * @return Pointer to the current scene, or nullptr if no scene is active
     */
    IScene* getCurrentScene() const { return _currentScene; }

private:
    IScene* _currentScene;                                    ///< Pointer to the currently active scene
    std::unordered_map<std::string, std::shared_ptr<IScene>> _scenes;  ///< Map of registered scenes
};
