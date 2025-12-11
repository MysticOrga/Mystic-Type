/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** How to add new scenes - Developer Guide
*/

/*
 * ============================================================================
 * HOW TO ADD NEW SCENES
 * ============================================================================
 *
 * The scene system is designed to be extensible and flexible. Follow these
 * steps to add a new scene to your game:
 *
 * ============================================================================
 * STEP 1: Create Your Scene Class
 * ============================================================================
 *
 * Create two files for your scene:
 * - src/graphical-client/Scenes/YourScene.hpp
 * - src/graphical-client/Scenes/YourScene.cpp
 *
 * Example: MenuScene.hpp
 * ──────────────────────────────────────────────────────────────────────────
 * 
 * #pragma once
 *
 * #include "IScene.hpp"
 * #include "../Raylib/Raylib.hpp"
 * #include "../../Network/Client/NetworkClient.hpp"
 * #include <memory>
 *
 * class MenuScene : public IScene {
 * public:
 *     MenuScene(Raylib::Window& window, NetworkClient& networkClient);
 *     ~MenuScene() = default;
 *
 *     // IScene interface implementation
 *     void onEnter() override;
 *     void onExit() override;
 *     void update(float dt) override;
 *     void render(float dt) override;
 *
 * private:
 *     Raylib::Window& _window;
 *     NetworkClient& _net;
 *     // Add your scene-specific members here
 * };
 *
 * ──────────────────────────────────────────────────────────────────────────
 *
 * Example: MenuScene.cpp (minimal implementation)
 * ──────────────────────────────────────────────────────────────────────────
 *
 * #include "MenuScene.hpp"
 * #include <iostream>
 *
 * MenuScene::MenuScene(Raylib::Window& window, NetworkClient& networkClient)
 *     : _window(window), _net(networkClient)
 * {
 * }
 *
 * void MenuScene::onEnter()
 * {
 *     std::cout << "[MenuScene] Entering menu scene\n";
 *     // Initialize scene resources here
 * }
 *
 * void MenuScene::onExit()
 * {
 *     std::cout << "[MenuScene] Exiting menu scene\n";
 *     // Clean up resources here
 * }
 *
 * void MenuScene::update(float dt)
 * {
 *     // Update scene logic
 *     // Handle input, update animations, etc.
 * }
 *
 * void MenuScene::render(float dt)
 * {
 *     _window.beginDrawing();
 *     _window.clearBackground(RAYWHITE);
 *
 *     // Render your menu here
 *     // Example: Draw buttons, text, etc.
 *
 *     _window.endDrawing();
 * }
 *
 * ──────────────────────────────────────────────────────────────────────────
 *
 * ============================================================================
 * STEP 2: Update CMakeLists.txt
 * ============================================================================
 *
 * Add your scene source files to src/graphical-client/CMakeLists.txt:
 *
 * set(CLIENT_SOURCES
 *     main.cpp
 *     GraphicClient/GraphicClient.cpp
 *     Raylib/Raylib.cpp
 *     Graphic/Graphic.cpp
 *     Scenes/SceneManager.cpp
 *     Scenes/GameScene.cpp
 *     Scenes/MenuScene.cpp          <-- Add your scene here
 *
 *     ../Network/Client/NetworkClient.cpp
 *     ../Network/TransportLayer/Packet.cpp
 *     ../Network/TransportLayer/ASocket.cpp
 *     ../Network/TransportLayer/UDP/UDPSocket.cpp
 * )
 *
 * ============================================================================
 * STEP 3: Register Your Scene in GraphicClient
 * ============================================================================
 *
 * Modify GraphicClient::init() in GraphicClient.cpp to register your scene:
 *
 * bool GraphicClient::init()
 * {
 *     if (!_net.connectToServer()) return false;
 *     if (!_net.performHandshake()) return false;
 *     std::cout << "[CLIENT] Assigned ID " << _net.getPlayerId() << "\n";
 *     _net.sendHelloUdp(0, 0);
 *
 *     _net.pollPackets();
 *
 *     // Create and register scenes
 *     auto gameScene = std::make_shared<GameScene>(_window, _net);
 *     auto menuScene = std::make_shared<MenuScene>(_window, _net);
 *
 *     _sceneManager.registerScene("game", gameScene);
 *     _sceneManager.registerScene("menu", menuScene);  <-- Add this
 *
 *     // Start with the game scene by default
 *     _sceneManager.changeScene("game");
 *
 *     _net.clearEvents();
 *     return true;
 * }
 *
 * ============================================================================
 * STEP 4: Switch Between Scenes
 * ============================================================================
 *
 * To switch from one scene to another (e.g., from GameScene to MenuScene),
 * you can access the SceneManager through GraphicClient or pass it as a
 * parameter to your scenes:
 *
 * Example in GameScene:
 * ──────────────────────────────────────────────────────────────────────────
 *
 * // To switch to menu when ESC is pressed:
 * if (IsKeyPressed(KEY_ESCAPE)) {
 *     sceneManager->changeScene("menu");
 * }
 *
 * ──────────────────────────────────────────────────────────────────────────
 *
 * ============================================================================
 * IScene Interface Methods
 * ============================================================================
 *
 * - onEnter():  Called when the scene becomes active
 * - onExit():   Called when the scene is being deactivated
 * - update(dt): Called every frame to update scene logic
 * - render(dt): Called every frame to render the scene
 *
 * ============================================================================
 * Best Practices
 * ============================================================================
 *
 * 1. Keep scene logic isolated within the scene class
 * 2. Use references to shared resources (Window, NetworkClient)
 * 3. Initialize resources in onEnter(), clean up in onExit()
 * 4. Keep render() focused on drawing only
 * 5. Use meaningful scene names for registration ("menu", "game", "lobby")
 * 6. Make scenes independent and reusable
 *
 */
