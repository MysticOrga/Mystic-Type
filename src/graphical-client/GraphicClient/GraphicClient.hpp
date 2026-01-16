/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** GraphicClient
*/

#pragma once

#include "../Raylib/Raylib.hpp"
#include "../ecs/Core.hpp"
#include "../ecs/System.hpp"
#include "../../Network/Client/NetworkClient.hpp"
#include "../../Network/Client/GameState.hpp"
#include <unordered_map>
#include <string>
#include <chrono>
#include <vector>

/**
* @class GraphicClient
* @brief Main client application managing graphics, networking, and game state.
*
* Handles game initialization, rendering, entity management, user input processing,
* menu interactions, and network communication with the game server.
*/
class GraphicClient {
public:
    /**
    * @brief Constructor initializing the graphic client.
    */
    GraphicClient();
    ~GraphicClient() = default;

    /**
    * @brief Main application loop.
    * @return Exit code (0 for success).
    */
    int run();
    
    static constexpr float GAME_AREA_SIZE = 765.0f;         /**< Game area render size in pixels */
    static constexpr float GAME_AREA_OFFSET_X = 577.5f;     /**< Game area X offset on screen */
    static constexpr float GAME_AREA_OFFSET_Y = 157.5f;     /**< Game area Y offset on screen */

private:
    /**
    * @brief Initializes the client (connects to server, shows menus).
    * @return True if initialization successful, false otherwise.
    */
    bool init();
    
    /**
    * @brief Main game loop iteration.
    */
    void gameLoop();
    
    /**
    * @brief Processes network events and updates game state.
    */
    void processNetworkEvents();
    
    /**
    * @brief Updates all game entities based on delta time.
    * @param dt Delta time since last update.
    */
    void updateEntities(float dt);
    
    /**
    * @brief Renders the entire game scene.
    * @param dt Delta time since last frame.
    */
    void render(float dt);
    
    /**
    * @brief Draws the game area background with animations.
    * @param hoverAnimTimer Animation timer for effects.
    */
    void drawGameBackground(float hoverAnimTimer);

    /**
    * @brief Creates a player entity at specified position.
    * @param x X position in game coordinates.
    * @param y Y position in game coordinates.
    * @return Entity ID of created player.
    */
    Entity createPlayerEntity(float x, float y);
    
    /**
    * @brief Creates a bullet entity at specified position.
    * @param x X position in game coordinates.
    * @param y Y position in game coordinates.
    * @param vx X velocity.
    * @param vy Y velocity.
    * @return Entity ID of created bullet.
    */
    Entity createBulletEntity(float x, float y, float vx, float vy);
    
    /**
    * @brief Creates a monster entity at specified position.
    * @param x X position in game coordinates.
    * @param y Y position in game coordinates.
    * @param type Monster type identifier.
    * @return Entity ID of created monster.
    */
    Entity createMonsterEntity(float x, float y, uint8_t type);

    /**
    * @brief Synchronizes player entities with server state.
    * @param players List of player states from server.
    */
    void syncEntities(const std::vector<PlayerState> &players);
    
    /**
    * @brief Synchronizes bullet entities with server state.
    * @param bullets List of bullet states from server.
    */
    void syncBullets(const std::vector<BulletState> &bullets);
    
    /**
    * @brief Synchronizes monster entities with server state.
    * @param monsters List of monster states from server.
    */
    void syncMonsters(const std::vector<MonsterState> &monsters);
    
    /**
    * @brief Shows lobby selection menu.
    * @return True if lobby selected successfully, false otherwise.
    */
    bool selectLobby();
    
    /**
    * @brief Shows pseudo/username input menu.
    * @return True if pseudo entered successfully, false otherwise.
    */
    bool selectPseudo();
    
    /**
    * @brief Shows main menu with server connection options.
    * @return True if user proceeded to game, false otherwise.
    */
    bool selectMainMenu();
    
    /**
    * @brief Shows controls settings menu for key remapping.
    * @return True on menu close, false on exit.
    */
    bool selectSettings();
    
    /**
    * @brief Converts keyboard key code to human-readable string.
    * @param key KeyboardKey code to convert.
    * @return String representation of the key.
    */
    std::string keyboardKeyToString(KeyboardKey key);
    
    /**
    * @brief Disconnects from server and quits the application.
    *
    * Properly closes network connections and sets exit flag.
    */
    void disconnectAndQuit();

    // Graphics and Rendering
    Raylib::Window _window;                          /**< Game window handle */
    
    // Networking
    NetworkClient _net;                             /**< Network client for server communication */
    GameState _state;                               /**< Current game state from server */
    
    // ECS Systems
    ECS _ecs;                                       /**< Main entity component system */
    ECS _uiEcs;                                     /**< UI entity component system */

    SpriteRenderSystem _spriteRenderSystem;         /**< Sprite rendering system */
    RectangleRenderSystem _rectangleRenderSystem;   /**< Rectangle rendering system */
    InputSystem _inputSystem;                       /**< Player input handling system */
    MovementSystem _movementSystem;                 /**< Entity movement system */

    // Entity Maps
    std::unordered_map<int, Entity> _entities;      /**< Mapping of server player IDs to entities */
    std::unordered_map<int, Entity> _bulletEntities; /**< Mapping of bullet IDs to entities */
    std::unordered_map<int, Entity> _monsterEntities; /**< Mapping of monster IDs to entities */
    
    // Chat System
    std::vector<std::string> _chatLog;              /**< Chat message history */
    std::string _chatInput;                         /**< Current chat input buffer */
    bool _chatActive = false;                       /**< Chat input active flag */
    
    // Player Info
    std::string _localPseudo;                       /**< Local player username */
    std::string _lastChatSent;                      /**< Last sent chat message */
    
    // Network Timing
    std::unordered_map<int, int> _playerPingMs;    /**< Ping values for each player */
    std::chrono::steady_clock::time_point _lastKeepAlive{}; /**< Last keep-alive packet time */
    std::chrono::steady_clock::time_point _lastHello{};     /**< Last hello packet time */
    std::chrono::steady_clock::time_point _lastUdpPing{};   /**< Last UDP ping time */
    bool _udpReady = false;                         /**< UDP connection ready flag */
    
    // Animation and State
    float _gameAnimTimer = 0.0f;                    /**< Global animation timer */
    bool _forceExit = false;                        /**< Force exit flag */
    bool _restartToMenu = false;                    /**< Restart to menu flag */
    bool _hasPseudo = false;                        /**< Player has entered pseudo flag */
    std::string _lastInitError;                     /**< Last initialization error message */
    
    // Server Connection
    std::string _serverIp = "127.0.0.1";            /**< Server IP address */
    int _serverPort = 4243;                         /**< Server port number */
    bool _hasServerConfig = false;                  /**< Server config provided flag */
    
    // Lobby Return
    bool _pendingReturnToLobby = false;             /**< Pending return to lobby flag */
    std::chrono::steady_clock::time_point _returnToLobbyAt{}; /**< Time to return to lobby */
    std::string _pendingReturnReason;               /**< Reason for returning to lobby */
};
