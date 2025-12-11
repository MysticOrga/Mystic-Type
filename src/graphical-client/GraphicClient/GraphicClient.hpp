/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** GraphicClient - Main graphics client application
*/

#pragma once

#include "../Raylib/Raylib.hpp"
#include "../Scenes/SceneManager.hpp"
#include "../../Network/Client/NetworkClient.hpp"
#include <string>
#include <memory>

/**
 * @class GraphicClient
 * @brief Main graphics client application
 * 
 * Manages the entire graphical client application including:
 * - Window initialization and management
 * - Scene management and transitions
 * - Network communication
 * - Game loop execution
 * 
 * The GraphicClient is responsible for coordinating between the Raylib window,
 * the network client, and the scene manager to provide a complete game experience.
 */
class GraphicClient {
public:
    /**
     * @brief Constructs a GraphicClient instance
     * @param ip The server IP address to connect to
     * @param port The server port to connect to
     */
    GraphicClient(const std::string& ip, int port);
    
    /**
     * @brief Destructor
     */
    ~GraphicClient() = default;

    /**
     * @brief Runs the main application loop
     * @return Exit code (0 for success, 1 for initialization failure)
     * 
     * This method initializes the client and starts the game loop.
     * It handles all network events, scene updates, and rendering.
     */
    int run();

private:
    /**
     * @brief Initializes the client
     * @return true if initialization was successful, false otherwise
     * 
     * Handles:
     * - Network server connection
     * - Handshake protocol
     * - Scene registration
     * - Initial scene setup
     */
    bool init();
    
    /**
     * @brief Main game loop
     * 
     * Continuously updates and renders scenes while the window is open.
     */
    void gameLoop();

    Raylib::Window _window;       ///< Raylib window instance
    NetworkClient _net;           ///< Network client for server communication
    SceneManager _sceneManager;   ///< Scene manager for handling scene transitions
};
