/*
** EPITECH PROJECT, 2025
** Mystic-Type
** File description:
** GraphicClient
*/

#include "graphical-client/GraphicClient/GraphicClient.hpp"
#include "graphical-client/Scenes/GameScene.hpp"
#include "graphical-client/Scenes/MenuScene.hpp"

#include <iostream>
#include <memory>

GraphicClient::GraphicClient(const std::string& ip, int port)
    : _window(1920, 1080, "Mystic-Type"), _net(ip, port)
{
    _window.setTargetFPS(60);
}

bool GraphicClient::init()
{
    if (!_net.connectToServer()) return false;
    if (!_net.performHandshake()) return false;
    std::cout << "[CLIENT] Assigned ID " << _net.getPlayerId() << "\n";
    _net.sendHelloUdp(0, 0);

    _net.pollPackets();

    // Create and register scenes
    auto gameScene = std::make_shared<GameScene>(_window, _net);
    auto menuScene = std::make_shared<MenuScene>(_window, _net);

    _sceneManager.registerScene("game", gameScene);
    _sceneManager.registerScene("menu", menuScene);

    // Set scene manager reference for all scenes
    gameScene->setSceneManager(&_sceneManager);
    menuScene->setSceneManager(&_sceneManager);

    // Optional: Register additional scenes here
    // auto lobbyScene = std::make_shared<LobbyScene>(_window, _net);
    // _sceneManager.registerScene("lobby", lobbyScene);
    // lobbyScene->setSceneManager(&_sceneManager);

    // Start with the menu scene by default
    _sceneManager.changeScene("menu");

    _net.clearEvents();
    return true;
}

void GraphicClient::gameLoop()
{
    while (!_window.shouldClose()) {
        float dt = _window.getFrameTime();
        _sceneManager.update(dt);
        _sceneManager.render(dt);
    }
}

int GraphicClient::run()
{
    if (!init()) return 1;
    gameLoop();
    return 0;
}

