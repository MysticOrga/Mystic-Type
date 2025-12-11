/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** GameScene - Main gameplay scene
*/

#pragma once

#include "IScene.hpp"
#include "../Raylib/Raylib.hpp"
#include "../ecs/Core.hpp"
#include "../ecs/System.hpp"
#include "../../Network/Client/NetworkClient.hpp"
#include "../../Network/Client/GameState.hpp"
#include <unordered_map>
#include <memory>

/**
 * @class GameScene
 * @brief Main gameplay scene with ECS-based entity management
 * 
 * Implements the core gameplay experience with:
 * - Entity-Component-System (ECS) architecture for game objects
 * - Network synchronization with server
 * - Input handling and player movement
 * - Sprite rendering for players, bullets, and monsters
 * 
 * The GameScene manages:
 * - Player entities and synchronization
 * - Bullet entities and their lifecycle
 * - Monster entities and their states
 * - Various rendering and movement systems
 */
class GameScene : public IScene {
public:
    /**
     * @brief Constructor for GameScene
     * @param window Reference to the Raylib window
     * @param networkClient Reference to the network client
     */
    GameScene(Raylib::Window& window, NetworkClient& networkClient);
    
    /**
     * @brief Destructor
     */
    ~GameScene() = default;

    // IScene interface implementation
    /**
     * @brief Called when entering the game scene
     * 
     * Initializes game state and synchronizes with server.
     */
    void onEnter() override;
    
    /**
     * @brief Called when exiting the game scene
     * 
     * Cleans up game resources.
     */
    void onExit() override;
    
    /**
     * @brief Updates game logic each frame
     * @param dt Delta time since last frame (in seconds)
     * 
     * Handles:
     * - Network event processing
     * - Entity synchronization
     * - Player input
     * - Movement updates
     */
    void update(float dt) override;
    
    /**
     * @brief Renders all game entities each frame
     * @param dt Delta time since last frame (in seconds)
     * 
     * Renders:
     * - Game area boundaries
     * - Player sprites
     * - Bullet graphics
     * - Monster sprites
     */
    void render(float dt) override;
    
    /**
     * @brief Sets the scene manager reference for scene transitions
     * @param sceneManager Pointer to the SceneManager
     */
    void setSceneManager(SceneManager* sceneManager) override { _sceneManager = sceneManager; }

private:
    // Reference members
    Raylib::Window& _window;           ///< Reference to the Raylib window
    NetworkClient& _net;               ///< Reference to the network client
    SceneManager* _sceneManager;       ///< Pointer to the scene manager (for transitions)

    // Scene state
    GameState _state;                  ///< Current game state
    ECS _ecs;                          ///< Entity Component System

    // Systems
    SpriteRenderSystem _spriteRenderSystem;         ///< Renders animated sprites
    RectangleRenderSystem _rectangleRenderSystem;   ///< Renders rectangles (bullets, monsters)
    InputSystem _inputSystem;                       ///< Handles player input
    MovementSystem _movementSystem;                 ///< Updates entity positions

    // Entity management
    std::unordered_map<int, Entity> _entities;        ///< Map of player entities by ID
    std::unordered_map<int, Entity> _bulletEntities;  ///< Map of bullet entities by ID
    std::unordered_map<int, Entity> _monsterEntities; ///< Map of monster entities by ID

    // Private methods
    /**
     * @brief Processes network events from the server
     */
    void processNetworkEvents();
    
    /**
     * @brief Updates all entities based on network and input data
     * @param dt Delta time since last frame
     */
    void updateEntities(float dt);

    /**
     * @brief Creates a player entity
     * @param x X coordinate
     * @param y Y coordinate
     * @return Entity ID of the created player
     */
    Entity createPlayerEntity(float x, float y);
    
    /**
     * @brief Creates a bullet entity
     * @param x X coordinate
     * @param y Y coordinate
     * @param vx Velocity X component
     * @param vy Velocity Y component
     * @return Entity ID of the created bullet
     */
    Entity createBulletEntity(float x, float y, float vx, float vy);
    
    /**
     * @brief Creates a monster entity
     * @param x X coordinate
     * @param y Y coordinate
     * @param type Monster type identifier
     * @return Entity ID of the created monster
     */
    Entity createMonsterEntity(float x, float y, uint8_t type);

    /**
     * @brief Synchronizes player entities with server state
     * @param players Vector of PlayerState from server
     */
    void syncEntities(const std::vector<PlayerState>& players);
    
    /**
     * @brief Synchronizes bullet entities with server state
     * @param bullets Vector of BulletState from server
     */
    void syncBullets(const std::vector<BulletState>& bullets);
    
    /**
     * @brief Synchronizes monster entities with server state
     * @param monsters Vector of MonsterState from server
     */
    void syncMonsters(const std::vector<MonsterState>& monsters);
};
