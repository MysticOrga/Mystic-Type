/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** IScene - Base interface for all scenes
*/

#pragma once

#include <memory>

class SceneManager;

/**
 * @class IScene
 * @brief Abstract base interface for all scene implementations
 * 
 * Defines the interface that all scenes must implement to work with the
 * SceneManager. A scene represents a distinct state or screen in the game,
 * such as menu, lobby, gameplay, or end screen.
 * 
 * @note All derived classes must implement the pure virtual methods.
 */
class IScene {
public:
    /**
     * @brief Virtual destructor
     */
    virtual ~IScene() = default;

    /**
     * @brief Called when the scene becomes the active scene
     * 
     * Use this method to initialize scene-specific resources,
     * such as loading assets, setting up entities, or initializing state.
     */
    virtual void onEnter() = 0;

    /**
     * @brief Called when the scene is being deactivated
     * 
     * Use this method to clean up resources and save any necessary state
     * before transitioning to another scene.
     */
    virtual void onExit() = 0;

    /**
     * @brief Updates the scene logic
     * 
     * Called every frame to update game logic, handle input, process
     * network events, and update entity states.
     * 
     * @param dt Delta time since the last frame (in seconds)
     */
    virtual void update(float dt) = 0;

    /**
     * @brief Renders the scene
     * 
     * Called every frame to render all visual elements of the scene.
     * This should include drawing backgrounds, entities, UI elements, etc.
     * 
     * @param dt Delta time since the last frame (in seconds)
     */
    virtual void render(float dt) = 0;

    /**
     * @brief Sets the scene manager reference (optional)
     * 
     * Called by SceneManager to provide a reference to itself.
     * This allows scenes to transition to other scenes.
     * 
     * @param sceneManager Pointer to the SceneManager instance
     * 
     * @note Only override this if your scene needs to change scenes.
     * The default implementation does nothing.
     */
    virtual void setSceneManager(SceneManager* sceneManager) {}
};

